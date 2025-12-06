#include "request_way.h"
#include "../nlohmann/json.hpp"
#include <filesystem>
#include <fstream>
#include <map>
#include <openssl/sha.h>
#include <regex>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <uuid/uuid.h>

extern std::map<std::string, std::string> users;
using json = nlohmann::json;
namespace fs = std::filesystem;
locker request_way::m_lock;

std::string generateUUID() {
  uuid_t uuid;
  uuid_generate_random(uuid);

  char uuid_str[37];
  uuid_unparse(uuid, uuid_str);

  return std::string(uuid_str);
}

std::string generateStoragePath(const std::string &file_id) {
  // 移除UUID中的横杠
  std::string clean_id;
  for (char c : file_id) {
    if (c != '-')
      clean_id += c;
  }
  // 取前4个字符构建两级目录
  // 例如：f47ac10b58cc4372a5670e02b2c3d479
  // dir1 = "f4", dir2 = "7a"
  std::string dir1 = clean_id.substr(0, 2);
  std::string dir2 = clean_id.substr(2, 2);
  // 完整路径：root/f4/7a/f47ac10b58cc4372a5670e02b2c3d479.bin
  return "./root/" + dir1 + "/" + dir2 + "/" + clean_id + ".bin";
}

void login_way::request_stratege(std::string &url, std::string &content,
                                 MYSQL *mysql, std::string &response_content) {
  std::string user;
  std::string password;
  json post_client = json::parse(content);
  json response_json;
  user = post_client["user"];
  password = post_client["passwd"];

  if (users.find(user) != users.end() && users[user] == password) {
    response_json["status"] = "success";
    response_content = response_json.dump();
  } else {
    response_json["status"] = "error";
    response_content = response_json.dump();
  }
}
static auto_register<login_way> login_auto_register;

void register_way::request_stratege(std::string &url, std::string &content,
                                    MYSQL *mysql,
                                    std::string &response_content) {
  std::regex pattern("^[a-zA-Z0-9]+$");
  std::string user;
  std::string password;
  json post_client = json::parse(content);
  json response_json;
  user = post_client["user"];
  password = post_client["passwd"];

  if (user.size() <= 1 || password.size() <= 1 || user.size() > 40 ||
      password.size() > 40 || !std::regex_match(user, pattern) ||
      !std::regex_match(password, pattern)) {
    response_json["status"] = "error";
    response_json["message"] = "user or password too long or too short,only "
                               "letters and numbers are allowed.";
    response_content = response_json.dump(4);
    return;
  }

  if (users.find(user) == users.end()) {
    std::stringstream insert_sql;
    insert_sql << "insert into user(username, passwd) values(";
    insert_sql << "'" << user << "', ";
    insert_sql << "'" << password << "')";

    m_lock.lock();
    int res = mysql_query(mysql, insert_sql.str().c_str());
    if (!res) {
      // 保证数据库和内存的一致性
      users.insert(pair<string, string>(user, password));
    }
    m_lock.unlock();

    if (res) {
      response_json["status"] = "error";
      response_json["message"] = "insertion error to sql";
      response_content = response_json.dump(4);
    } else {
      response_json["status"] = "success";
      response_content = response_json.dump(4);
    }

  } else {
    response_json["status"] = "error";
    response_json["message"] = "existing account";
    response_content = response_json.dump(4);
  }
}
static auto_register<register_way> register_auto_register;

// 文件操作相关策略实现

void file_download_way::request_stratege(std::string &url, std::string &content,
                                         MYSQL *mysql,
                                         std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<file_download_way> file_download_auto_register;

void file_upload_way::request_stratege(std::string &url, std::string &content,
                                       MYSQL *mysql,
                                       std::string &response_content) {

  // 文件上传逻辑
  json post_client = json::parse(content);
  json response_json;
  std::string username = post_client["username"];
  std::string file_size = post_client["file_size"];
  std::string virtual_file_path = post_client["virtual_file_path"];
  std::string document_content = post_client["document_content"];
  std::string actual_file_path;
  std::string sha256_num = post_client["sha256_num"];
  std::string file_id;
  std::stringstream select_file_table_sql;

  select_file_table_sql
      << "select file_id,sha256_num from file_table where sha256_num = '"
      << sha256_num << "'";
  m_lock.lock();
  int res = mysql_query(mysql, select_file_table_sql.str().c_str());
  m_lock.unlock();
  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    response_content = response_json.dump(4);
    return;
  } else {
    MYSQL_RES *result = mysql_store_result(mysql);

    file_id =
        result->row_count > 0 ? mysql_fetch_row(result)[0] : generateUUID();

    if (result->row_count > 0) {
      // 文件存在,增加引用次数,并向联系表增加一个记录
      std::stringstream update_file_table_sql;
      std::stringstream insert_own_table_sql;
      update_file_table_sql
          << "update file_table set citation_count =citation_count  + 1 where "
             "sha256_num = '"
          << sha256_num << "'";
      insert_own_table_sql
          << "insert into own_table(file_id, virtual_file_path, "
             "username) values('"
          << file_id << "', '" << virtual_file_path << "', '" << username
          << "')";
      m_lock.lock();
      // 开始事务
      res = mysql_query(mysql, "START TRANSACTION");
      if (res) {
        m_lock.unlock();
        response_json["status"] = "error";
        response_json["message"] = "启动事务失败";
        response_content = response_json.dump(4);
        mysql_free_result(result);
        return;
      }
      // 执行更新
      res = mysql_query(mysql, update_file_table_sql.str().c_str());
      if (res) {
        mysql_query(mysql, "ROLLBACK");
        m_lock.unlock();
        response_json["status"] = "error";
        response_json["message"] = "SQL更新错误";
        response_content = response_json.dump(4);
        mysql_free_result(result);
        return;
      }
      // 执行插入
      res = mysql_query(mysql, insert_own_table_sql.str().c_str());
      if (res) {
        mysql_query(mysql, "ROLLBACK");
        m_lock.unlock();
        response_json["status"] = "error";
        response_json["message"] = "SQL插入错误";
        response_content = response_json.dump(4);
        mysql_free_result(result);
        return;
      }
      // 提交事务
      res = mysql_query(mysql, "COMMIT");
      if (res) {
        mysql_query(mysql, "ROLLBACK");
        m_lock.unlock();
        response_json["status"] = "error";
        response_json["message"] = "提交事务失败";
        response_content = response_json.dump(4);
        mysql_free_result(result);
        return;
      }
      m_lock.unlock();
      response_json["status"] = "success";
      response_json["message"] = "file already exists, citation count "
                                 "increased, virtual path added";
      response_content = response_json.dump(4);
      return;
    }
    mysql_free_result(result);
  }
  // 文件不存在,保存文件,并向文件表和联系表增加记录
  actual_file_path = generateStoragePath(file_id);
  std::stringstream insert_file_table_sql;
  std::stringstream insert_own_table_sql;
  insert_file_table_sql
      << "insert into file_table(file_id, file_size,actual_file_path, "
         "citation_count,sha256_num) "
      << "values('" << file_id << "', '" << file_size << "', '"
      << actual_file_path << "', 1, '" << sha256_num << "')";
  insert_own_table_sql << "insert into own_table(file_id, virtual_file_path, "
                          "username) values('"
                       << file_id << "', '" << virtual_file_path << "', '"
                       << username << "')";
  m_lock.lock();
  // 开始事务
  res = mysql_query(mysql, "START TRANSACTION");
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "启动事务失败";
    response_content = response_json.dump(4);
    return;
  }
  res = mysql_query(mysql, insert_file_table_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql插入错误";
    response_content = response_json.dump(4);
    return;
  }
  // 执行插入
  res = mysql_query(mysql, insert_own_table_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "SQL插入错误";
    response_content = response_json.dump(4);
    return;
  }
  // 提交事务
  res = mysql_query(mysql, "COMMIT");
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "提交事务失败";
    response_content = response_json.dump(4);
    return;
  }
  m_lock.unlock();

  fs::path file_path;

  fs::create_directories(file_path.parent_path());

  std::ofstream output_file(actual_file_path, std::ios::binary);
  if (!output_file.is_open()) {
    response_json["status"] = "error";
    response_json["message"] = "file creation failed";
    response_content = response_json.dump(4);
    return;
  }
  output_file.write(document_content.c_str(), document_content.size());
  output_file.close();
  response_json["status"] = "success";
  response_content = response_json.dump(4);
}
static auto_register<file_upload_way> file_upload_auto_register;

void file_delete_way::request_stratege(std::string &url, std::string &content,
                                       MYSQL *mysql,
                                       std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<file_delete_way> file_delete_auto_register;

void file_rename_way::request_stratege(std::string &url, std::string &content,
                                       MYSQL *mysql,
                                       std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<file_rename_way> file_rename_auto_register;

void file_move_way::request_stratege(std::string &url, std::string &content,
                                     MYSQL *mysql,
                                     std::string &response_content) {

  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<file_move_way> file_move_auto_register;

void file_copy_way::request_stratege(std::string &url, std::string &content,
                                     MYSQL *mysql,
                                     std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<file_copy_way> file_copy_auto_register;

// 目录操作相关策略实现
void directory_create_way::request_stratege(std::string &url,
                                            std::string &content, MYSQL *mysql,
                                            std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<directory_create_way> directory_create_auto_register;

void directory_delete_way::request_stratege(std::string &url,
                                            std::string &content, MYSQL *mysql,
                                            std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<directory_delete_way> directory_delete_auto_register;

void directory_list_way::request_stratege(std::string &url,
                                          std::string &content, MYSQL *mysql,
                                          std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<directory_list_way> directory_list_auto_register;

void directory_rename_way::request_stratege(std::string &url,
                                            std::string &content, MYSQL *mysql,
                                            std::string &response_content) {
  std::string user;
  json post_client = json::parse(content);
  user = post_client["user"];
}
static auto_register<directory_rename_way> directory_rename_auto_register;
