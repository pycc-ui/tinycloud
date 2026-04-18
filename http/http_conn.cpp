#include "http_conn.h"
#include "request_way.h"

#include <fcntl.h>
#include <mysql/mysql.h>
#include <mysql/mysql_version.h>
#include <string>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <utility>

// 定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form =
    "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form =
    "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form =
    "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form =
    "There was an unusual problem serving the request file.\n";

map<string, string> users;

void http_conn::initmysql_result(connection_pool *connPool) {
  // 先从连接池中取一个连接
  MYSQL *mysql = NULL;
  // 该对象自动管理申请和释放
  connectionRAII mysqlcon(&mysql, connPool);

  // 在user表中检索username，passwd数据，浏览器端输入
  if (mysql_query(mysql, "SELECT username,passwd FROM user_table")) {
    LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
  }

  // 从表中检索完整的结果集
  MYSQL_RES *result = mysql_store_result(mysql);

  // 返回结果集中的列数
  int num_fields = mysql_num_fields(result);

  // 返回所有字段结构的数组
  MYSQL_FIELD *fields = mysql_fetch_fields(result);

  // 从结果集中获取下一行，将对应的用户名和密码，存入map中
  while (MYSQL_ROW row = mysql_fetch_row(result)) {
    string temp1(row[0]);
    string temp2(row[1]);
    users[temp1] = temp2;
  }
}

// 对文件描述符设置非阻塞
int setnonblocking(int fd) {
  int old_option = fcntl(fd, F_GETFL);
  int new_option = old_option | O_NONBLOCK;
  fcntl(fd, F_SETFL, new_option);
  return old_option;
}

// 将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
void addfd(int epollfd, int fd, bool one_shot, int TRIGMode) {
  epoll_event event;
  event.data.fd = fd;

  if (1 == TRIGMode)
    event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
  else
    event.events = EPOLLIN | EPOLLRDHUP;

  if (one_shot)
    event.events |= EPOLLONESHOT;
  epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
  setnonblocking(fd);
}

// 从内核时间表删除描述符
void removefd(int epollfd, int fd) {
  epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, 0);
  close(fd);
}

// 将事件重置为EPOLLONESHOT
void modfd(int epollfd, int fd, int ev, int TRIGMode) {
  epoll_event event;
  event.data.fd = fd;

  // TRIGMode是选择边缘触发还是条件触发
  if (1 == TRIGMode)
    event.events = ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
  else
    event.events = ev | EPOLLONESHOT | EPOLLRDHUP;

  epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

int http_conn::m_user_count = 0;
int http_conn::m_epollfd = -1;

// 关闭连接，关闭一个连接，客户总量减一
void http_conn::close_conn(bool real_close) {
  if (real_close && (m_sockfd != -1)) {
    LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
             (unsigned long)pthread_self(), "close fd");
    removefd(m_epollfd, m_sockfd);
    m_sockfd = -1;
    m_user_count--;
  }
}

// 初始化连接,外部调用初始化套接字地址
void http_conn::init(int sockfd, const sockaddr_in &addr, char *root,
                     int TRIGMode, int close_log, string user, string passwd,
                     string sqlname) {
  m_sockfd = sockfd;
  m_address = addr;

  addfd(m_epollfd, sockfd, true, m_TRIGMode);
  m_user_count++;

  // 当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
  doc_root = root;
  m_TRIGMode = TRIGMode;
  m_close_log = close_log;
  m_linger = true;

  strcpy(sql_user, user.c_str());
  strcpy(sql_passwd, passwd.c_str());
  strcpy(sql_name, sqlname.c_str());

  init_read();
  init_write();
}

// 初始化新接受的连接
// check_state默认为分析请求行状态
void http_conn::init_read() {
  mysql = NULL;
  m_check_state = CHECK_STATE_REQUESTLINE;
  m_method = GET;
  m_url = 0;
  m_version = 0;
  m_content_length = 0;
  m_host = 0;
  m_start_line = 0;
  m_checked_idx = 0;
  m_read_idx = 0;
  m_state = 0;
  timer_flag = 0;
  improv = 0;
  m_read_message = std::make_unique<json>();
  memset(m_read_buf, '\0', READ_BUFFER_SIZE);
}

void http_conn::init_write() {
  m_response_content = "";
  m_write_idx = 0;
  bytes_to_send = 0;
  bytes_have_send = 0;
  memset(m_write_buf, '\0', WRITE_BUFFER_SIZE);
}

// 循环读取客户数据，直到无数据可读或对方关闭连接
// 非阻塞ET工作模式下，需要一次性将数据读完
// 读取所有数据,m_read_idx作为读到该位置的标志
bool http_conn::read_once() {

  if (m_read_idx >= READ_BUFFER_SIZE) {
    LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
             (unsigned long)pthread_self(),
             "从内核读取超过出缓冲区:m_read_idx >= READ_BUFFER_SIZE");
    return false;
  }

  int bytes_read = 0;

  // LT读取数据
  if (0 == m_TRIGMode) {

    bytes_read = recv(m_sockfd, m_read_buf + m_read_idx,
                      READ_BUFFER_SIZE - m_read_idx, 0);
    m_read_idx += bytes_read;

    if (bytes_read <= 0) {
      LOG_INFO("[%s:%d][%s][Thread:%lx]:%s %s", __FILE__, __LINE__, __func__,
               (unsigned long)pthread_self(), "recv返回有问题",
               strerror(errno));
      return false;
    }

    return true;
  }

  // ET读数据
  else {
    // 不采用
    while (true) {
      bytes_read = recv(m_sockfd, m_read_buf + m_read_idx,
                        READ_BUFFER_SIZE - m_read_idx, 0);
      if (bytes_read == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
          break;
        return false;
      } else if (bytes_read == 0) {
        return false;
      }
      m_read_idx += bytes_read;
    }
    return true;
  }
}

// 从状态机，用于分析出一行内容
// 返回值为行的读取状态，有LINE_OK,LINE_BAD,LINE_OPEN
// 功能是处理每一行的\r\n,将其变成\0\0
// 请求行的每一行结尾都有\r\n
// m_checked_idx作为已处理的标志
/*
 三种状态，标识解析一行的读取状态。

LINE_OK，完整读取一行

LINE_BAD，报文语法有误

LINE_OPEN，读取的行不完整
 */
http_conn::LINE_STATUS http_conn::parse_line() {
  char temp;
  for (; m_checked_idx < m_read_idx; ++m_checked_idx) {
    temp = m_read_buf[m_checked_idx];
    if (temp == '\r') {
      if ((m_checked_idx + 1) == m_read_idx)
        return LINE_OPEN;
      else if (m_read_buf[m_checked_idx + 1] == '\n') {
        m_read_buf[m_checked_idx++] = '\0';
        m_read_buf[m_checked_idx++] = '\0';
        return LINE_OK;
      }
      return LINE_BAD;
    } else if (temp == '\n') {
      if (m_checked_idx > 1 && m_read_buf[m_checked_idx - 1] == '\r') {
        m_read_buf[m_checked_idx - 1] = '\0';
        m_read_buf[m_checked_idx++] = '\0';
        return LINE_OK;
      }
      return LINE_BAD;
    }
  }
  return LINE_OPEN;
}

// 解析http请求行，获得请求方法，目标url及http版本号
/*
NO_REQUEST

请求不完整，需要继续读取请求报文数据

GET_REQUEST

获得了完整的HTTP请求

BAD_REQUEST

HTTP请求报文有语法错误

INTERNAL_ERROR

服务器内部错误，该结果在主状态机逻辑switch的default下，一般不会触发
*/

http_conn::HTTP_CODE http_conn::parse_request_line(char *text) {
  // 查找\t或空格开头的指针
  // 请求行写入日志
  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), text);
  m_url = strpbrk(text, " \t");
  if (!m_url) {
    return BAD_REQUEST;
  }
  // 截断,前面的部分就是方法
  *m_url++ = '\0';
  char *method = text;
  // 只有GET和POST方法
  if (strcasecmp(method, "GET") == 0) {
  } else if (strcasecmp(method, "POST") == 0) {
  } else
    return BAD_REQUEST;

  // 跳过可能的连续空白
  m_url += strspn(m_url, " \t");
  m_version = strpbrk(m_url, " \t");
  if (!m_version)
    return BAD_REQUEST;
  // m_version指向版本部分
  *m_version++ = '\0';
  // 跳过可能的连续空白
  m_version += strspn(m_version, " \t");
  // 使用的是http1.1,非http1.1不接受
  if (strcasecmp(m_version, "HTTP/1.1") != 0)
    return BAD_REQUEST;
  // 指向uri
  if (strncasecmp(m_url, "http://", 7) == 0) {
    m_url += 7;
    m_url = strchr(m_url, '/');
  }
  // 指向uri
  if (strncasecmp(m_url, "https://", 8) == 0) {
    m_url += 8;
    // 找到主机名后的第一个/,/后面就是路径名了
    m_url = strchr(m_url, '/');
  }

  if (!m_url || m_url[0] != '/')
    return BAD_REQUEST;
  // 转入解析请求头部
  (*m_read_message)["url"] = m_url;

  m_check_state = CHECK_STATE_HEADER;
  return NO_REQUEST;
}

// 解析http请求的一个头部信息
http_conn::HTTP_CODE http_conn::parse_headers(char *text) {
  // 头部写入日志
  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), text);
  if (text[0] == '\0') {
    if (m_content_length != 0) {
      // 检查请求体的长度如果不为0,将转入处理消息体
      m_check_state = CHECK_STATE_CONTENT;
      return NO_REQUEST;
    }
    return GET_REQUEST;
  } else if (strncasecmp(text, "Connection:", 11) == 0) {
    text += 11;
    text += strspn(text, " \t");
    if (strcasecmp(text, "keep-alive") == 0) {
      m_linger = true;
    } else {
      m_linger = false;
    }
  } else if (strncasecmp(text, "Content-length:", 15) == 0) {
    text += 15;
    text += strspn(text, " \t");
    m_content_length = atol(text);
  } else if (strncasecmp(text, "Host:", 5) == 0) {
    text += 5;
    text += strspn(text, " \t");
    m_host = text;
  } else {
    // text = get_line()会将下面没处理的头部跳过
    LOG_INFO("oop!unknow header: %s", text);
  }
  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::parse_content(char *text) {
  // 请求体写入日志（只打印前100和后100字节）
  std::string content(text);
  std::string log_content;
  if (content.length() > 200) {
    log_content = content.substr(0, 100) + "... [中间省略 " +
                  std::to_string(content.length() - 200) + " 字节] ..." +
                  content.substr(content.length() - 100);
  } else {
    log_content = content;
  }
  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), log_content.c_str());

  LOG_INFO("[%s:%d] m_read_idx=%d, m_content_length=%d, m_checked_idx=%d, "
           "total_needed=%d",
           __FILE__, __LINE__, m_read_idx, m_content_length, m_checked_idx,
           m_content_length + m_checked_idx);

  // 消息体前有一个/r/n被处理
  if (m_read_idx >= (m_content_length + m_checked_idx)) {
    text[m_content_length] = '\0';
    m_string = text;
    (*m_read_message)["client_content"] = string(m_string);
    return GET_REQUEST;
  }
  return NO_REQUEST;
}

http_conn::HTTP_CODE http_conn::process_read() {
  LINE_STATUS line_status = LINE_OK;
  HTTP_CODE ret = NO_REQUEST;
  char *text = 0;

  while ((m_check_state == CHECK_STATE_CONTENT && line_status == LINE_OK) ||
         ((line_status = parse_line()) == LINE_OK)) {
    text = get_line();
    // 开始的行在m_checked_idx之前的m_checked_idx开始
    // 而m_checked_idx的位置在两个空字符后面(或者没有)
    // while的条件能保证getline每次都能成功
    // 读消息体时不会触发parse_line
    m_start_line = m_checked_idx;
    switch (m_check_state) {
    case CHECK_STATE_REQUESTLINE: {
      ret = parse_request_line(text);
      if (ret == BAD_REQUEST)
        return BAD_REQUEST;
      break;
    }
    case CHECK_STATE_HEADER: {
      ret = parse_headers(text);
      if (ret == BAD_REQUEST)
        return BAD_REQUEST;
      else if (ret == GET_REQUEST) {
        return do_request();
      }
      break;
    }
    case CHECK_STATE_CONTENT: {
      ret = parse_content(text);
      if (ret == GET_REQUEST)
        return do_request();
      return NO_REQUEST;
      break;
    }
    default:
      return INTERNAL_ERROR;
    }
  }
  return NO_REQUEST;
}

// 处理文件和文件目录信息
// put,get,post,,方法
// cgi处理数据库
http_conn::HTTP_CODE http_conn::do_request() {

  way_manager solve_request(m_close_log);
  solve_request.do_way(mysql, m_read_message);
  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "处理好了报文");
  return FILE_REQUEST;
}

bool http_conn::write() {
  int temp = 0;

  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "写缓冲区:", m_write_buf);

  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(),
           "回复报文:", m_response_content.c_str());
  if (bytes_to_send == 0) {
    return true;
  }

  while (1) {
    temp = writev(m_sockfd, m_iv, m_iv_count);

    if (temp < 0) {
      if (errno == EAGAIN) {
        // 资源不可用,意思是稍后试试
        modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
        LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
                 (unsigned long)pthread_self(), "资源不可用");
        return true;
      }
      m_response_content.clear();
      LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
               (unsigned long)pthread_self(), "写错误,writev为-1");
      return false;
    }

    bytes_have_send += temp;
    bytes_to_send -= temp;
    if (bytes_have_send >= m_iv[0].iov_len) {
      m_iv[0].iov_len = 0;
      m_iv[1].iov_base =
          &m_response_content[0] + (bytes_have_send - m_write_idx);
      m_iv[1].iov_len = bytes_to_send;
    } else {
      m_iv[0].iov_base = m_write_buf + bytes_have_send;
      m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
    }

    if (bytes_to_send <= 0) {
      m_response_content.clear();
      LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
               (unsigned long)pthread_self(), "写完毕");
      return true;
    }
  }
}
// 添加回复报文

bool http_conn::add_response(const char *format, ...) {
  if (m_write_idx >= WRITE_BUFFER_SIZE)
    return false;
  va_list arg_list;
  va_start(arg_list, format);
  int len = vsnprintf(m_write_buf + m_write_idx,
                      WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
  if (len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx)) {
    va_end(arg_list);
    return false;
  }
  m_write_idx += len;
  va_end(arg_list);

  return true;
}
// 增加状态行
bool http_conn::add_status_line(int status, const char *title) {
  return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
// 增加头部
bool http_conn::add_headers(int content_len) {
  return add_content_length(content_len) && add_linger() && add_blank_line();
}
bool http_conn::add_content_length(int content_len) {
  return add_response("Content-Length:%d\r\n", content_len);
}
bool http_conn::add_content_type() {
  // 这里需要变一下或者直接删除
  return add_response("Content-Type:%s\r\n", "application/json");
}
bool http_conn::add_linger() {
  return add_response("Connection:%s\r\n",
                      (m_linger == true) ? "keep-alive" : "close");
}
bool http_conn::add_blank_line() { return add_response("%s", "\r\n"); }
bool http_conn::add_content(const char *content) {
  return add_response("%s", content);
}

bool http_conn::process_write(HTTP_CODE ret) {
  switch (ret) {
  case INTERNAL_ERROR: {
    add_status_line(500, error_500_title);
    add_headers(strlen(error_500_form));
    if (!add_content(error_500_form))
      return false;
    break;
  }
  case BAD_REQUEST: {
    add_status_line(404, error_404_title);
    add_headers(strlen(error_404_form));
    if (!add_content(error_404_form))
      return false;
    break;
  }
  case FORBIDDEN_REQUEST: {
    add_status_line(403, error_403_title);
    add_headers(strlen(error_403_form));
    if (!add_content(error_403_form))
      return false;
    break;
  }
  case FILE_REQUEST: {
    add_status_line(200, ok_200_title);
    if (m_response_content.size() != 0) {
      add_headers(m_response_content.size());
      // 写缓冲区
      m_iv[0].iov_base = m_write_buf;
      m_iv[0].iov_len = m_write_idx;
      // 发送的文件
      m_iv[1].iov_base = &m_response_content[0];
      m_iv[1].iov_len = m_response_content.size();
      m_iv_count = 2;
      bytes_to_send = m_write_idx + m_response_content.size();
      return true;
    } else {
      // 没有发文件就发一个ok报文
      const char *ok_string = "ok";
      add_headers(strlen(ok_string));
      if (!add_content(ok_string))
        return false;
    }
  }
  default:
    return false;
  }
  m_iv[0].iov_base = m_write_buf;
  m_iv[0].iov_len = m_write_idx;
  m_iv_count = 1;
  bytes_to_send = m_write_idx;
  return true;
}

void http_conn::process_read_phase() {

  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "读开始");

  HTTP_CODE read_ret = process_read();
  if (read_ret == NO_REQUEST) {
    LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
             (unsigned long)pthread_self(), "没有读完继续读");
    modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
    return;
  }
  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "读完毕");
  (*m_read_message)["read_ret"] = static_cast<int>(read_ret);

  m_write_message = std::move(m_read_message);

  init_write();
  modfd(m_epollfd, m_sockfd, EPOLLOUT, m_TRIGMode);
}

bool http_conn::process_write_phase() {

  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "写开始");

  HTTP_CODE read_ret =
      static_cast<HTTP_CODE>((*m_write_message)["read_ret"].get<int>());
  m_response_content = (*m_write_message)["server_content"];

  bool write_ret = process_write(read_ret);

  if (!write_ret) {
    write();
    close_conn();
    LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
             (unsigned long)pthread_self(), "write_ret指示断开连接");
    return false;
  }
  bool write_result = write();

  LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
           (unsigned long)pthread_self(), "write退出");
  if (m_linger) {
    init_read();
    modfd(m_epollfd, m_sockfd, EPOLLIN, m_TRIGMode);
  } else {
    LOG_INFO("[%s:%d][%s][Thread:%lx]:%s", __FILE__, __LINE__, __func__,
             (unsigned long)pthread_self(), "断开连接");
    return false;
  }
  return write_result;
}
