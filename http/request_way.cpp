#include "request_way.h"
#include "../base64/base64.h"
#include "../nlohmann/json.hpp"

#include <filesystem>
#include <fstream>
#include <ios>
#include <map>
#include <mysql/mysql.h>
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

void login_way::request_stratege(MYSQL *mysql,
                                 std::unique_ptr<json> &message_json) {
  std::string user;
  std::string password;
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  json response_json;
  message_json->erase("client_content");
  user = post_client["user"];
  password = post_client["passwd"];

  if (users.find(user) != users.end() && users[user] == password) {
    response_json["status"] = "success";
  } else {
    response_json["status"] = "error";
    response_json["message"] = "invalid username or password";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }
}
static auto_register<login_way> login_auto_register;

void register_way::request_stratege(MYSQL *mysql,
                                    std::unique_ptr<json> &message_json) {
  std::regex pattern("^[a-zA-Z0-9]+$");
  std::string user;
  std::string password;
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;
  user = post_client["user"];
  password = post_client["passwd"];

  if (user.size() <= 1 || password.size() <= 1 || user.size() > 40 ||
      password.size() > 40 || !std::regex_match(user, pattern) ||
      !std::regex_match(password, pattern)) {
    response_json["status"] = "error";
    response_json["message"] = "user or password too long or too short,only "
                               "letters and numbers are allowed.";
    (*message_json)["server_content"] = response_json.dump(4);
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
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    } else {
      response_json["status"] = "success";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
  } else {
    response_json["status"] = "error";
    response_json["message"] = "existing account";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }
}
static auto_register<register_way> register_auto_register;

// 文件操作相关策略实现

void file_download_way::request_stratege(MYSQL *mysql,
                                         std::unique_ptr<json> &message_json) {
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  bool downloading = post_client["downloading"];
  if (downloading) {
    string actual_file_path = post_client["actual_file_path"];
    string username = post_client["username"];
    int block_size = atoi(std::string(post_client["block_size"]).c_str());
    int block_begin = atoi(std::string(post_client["block_begin"]).c_str());
    string document_content;
    std::ifstream input_file(actual_file_path, std::ios::binary);
    if (!input_file.is_open()) {
      response_json["status"] = "error";
      response_json["message"] = "file open failed";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
    input_file.seekg(block_begin, ios_base::beg);
    document_content.resize(block_size);
    input_file.read(document_content.data(), block_size);
    document_content = base64_encode(document_content);
    input_file.close();

    response_json["status"] = "success";
    response_json["message"] = "file block downloading";
    response_json["document_content"] = document_content;
    (*message_json)["server_content"] = response_json.dump(4);

    return;
  } else {
    std::string username = post_client["username"];
    std::string virtual_file_path = post_client["virtual_file_path"];
    string actual_file_path;
    std::stringstream select_file_table_sql;

    select_file_table_sql
        << "select actual_file_path from file_table where file_id = (";
    select_file_table_sql << "select file_id from own_table where  username= '"
                          << username << "' and virtual_file_path = '"
                          << virtual_file_path << "');";
    m_lock.lock();
    int res = mysql_query(mysql, select_file_table_sql.str().c_str());
    m_lock.unlock();
    if (res) {
      response_json["status"] = "error";
      response_json["message"] = "sql query error";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    } else {
      MYSQL_RES *result = mysql_store_result(mysql);
      if (result->row_count > 0) {
        MYSQL_ROW row = mysql_fetch_row(result);
        actual_file_path = row[0];
        mysql_free_result(result);
      } else {
        response_json["status"] = "error";
        response_json["message"] = "sql query error";
        (*message_json)["server_content"] = response_json.dump(4);
        return;
      }
      response_json["status"] = "success";
      response_json["message"] = "file download begin";
      response_json["actual_file_path"] = actual_file_path;
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
  }
}
static auto_register<file_download_way> file_download_auto_register;

void file_upload_way::request_stratege(MYSQL *mysql,
                                       std::unique_ptr<json> &message_json) {
  // 文件上传逻辑
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  bool appanding = post_client["appanding"];

  if (appanding) {
    std::string actual_file_path = post_client["actual_file_path"];
    std::string document_content = post_client["document_content"];
    // 目录存在时不会报错

    std::ofstream output_file(actual_file_path, std::ios::app);
    if (!output_file.is_open()) {
      response_json["status"] = "error";
      response_json["message"] = "file open failed";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }

    document_content = base64_decode(document_content);
    output_file.write(document_content.c_str(), document_content.size());
    output_file.close();

    response_json["status"] = "success";
    response_json["message"] = "file block uploaded successfully";
    (*message_json)["server_content"] = response_json.dump(4);
    return;

  } else {
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
        << sha256_num << "';";
    m_lock.lock();
    int res = mysql_query(mysql, select_file_table_sql.str().c_str());
    m_lock.unlock();
    if (res) {
      response_json["status"] = "error";
      response_json["message"] = "sql query error";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    } else {
      MYSQL_RES *result = mysql_store_result(mysql);

      file_id =
          result->row_count > 0 ? mysql_fetch_row(result)[0] : generateUUID();

      if (result->row_count > 0) {
        // 文件存在,增加引用次数,并向联系表增加一个记录
        std::stringstream update_file_table_sql;
        std::stringstream insert_own_table_sql;
        update_file_table_sql << "update file_table set citation_count "
                                 "=citation_count  + 1 where "
                                 "sha256_num = '"
                              << sha256_num << "';";
        insert_own_table_sql
            << "insert into own_table(file_id, virtual_file_path, "
               "username) values('"
            << file_id << "', '" << virtual_file_path << "', '" << username
            << "');";
        m_lock.lock();
        // 开始事务
        res = mysql_query(mysql, "START TRANSACTION");
        if (res) {
          m_lock.unlock();
          response_json["status"] = "error";
          response_json["message"] = "启动事务失败";
          (*message_json)["server_content"] = response_json.dump(4);
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
          (*message_json)["server_content"] = response_json.dump(4);
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
          (*message_json)["server_content"] = response_json.dump(4);
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
          (*message_json)["server_content"] = response_json.dump(4);
          mysql_free_result(result);
          return;
        }
        m_lock.unlock();
        response_json["status"] = "success";
        response_json["message"] = "file already exists, citation count "
                                   "increased, virtual path added";
        (*message_json)["server_content"] = response_json.dump(4);
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
        << actual_file_path << "', 1, '" << sha256_num << "');";
    insert_own_table_sql << "insert into own_table(file_id, virtual_file_path, "
                            "username) values('"
                         << file_id << "', '" << virtual_file_path << "', '"
                         << username << "');";
    m_lock.lock();
    // 开始事务
    res = mysql_query(mysql, "START TRANSACTION");
    if (res) {
      m_lock.unlock();
      response_json["status"] = "error";
      response_json["message"] = "启动事务失败";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
    res = mysql_query(mysql, insert_file_table_sql.str().c_str());
    if (res) {
      mysql_query(mysql, "ROLLBACK");
      m_lock.unlock();
      response_json["status"] = "error";
      response_json["message"] = "sql插入错误";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
    // 执行插入
    res = mysql_query(mysql, insert_own_table_sql.str().c_str());
    if (res) {
      mysql_query(mysql, "ROLLBACK");
      m_lock.unlock();
      response_json["status"] = "error";
      response_json["message"] = "SQL插入错误";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
    // 提交事务
    res = mysql_query(mysql, "COMMIT");
    if (res) {
      mysql_query(mysql, "ROLLBACK");
      m_lock.unlock();
      response_json["status"] = "error";
      response_json["message"] = "提交事务失败";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }
    m_lock.unlock();

    fs::path file_path;
    file_path = actual_file_path;
    fs::create_directories(file_path.parent_path());
    response_json["status"] = "success";
    response_json["message"] = "file uploaded begin";
    response_json["actual_file_path"] = actual_file_path;
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }
}
static auto_register<file_upload_way> file_upload_auto_register;

void file_delete_way::request_stratege(MYSQL *mysql,
                                       std::unique_ptr<json> &message_json) {
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string virtual_file_path = post_client["virtual_file_path"];
  std::string username = post_client["username"];

  // 1. 先查询要删除的文件ID
  std::stringstream select_sql;
  select_sql << "SELECT file_id FROM own_table WHERE username = '" << username
             << "' AND virtual_file_path = '" << virtual_file_path << "';";

  m_lock.lock();
  int res = mysql_query(mysql, select_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql select error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  if (result == nullptr || result->row_count == 0) {
    if (result)
      mysql_free_result(result);
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "file not found";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_ROW row = mysql_fetch_row(result);
  std::string file_id = row[0];
  mysql_free_result(result);

  // 2. 开始事务
  res = mysql_query(mysql, "START TRANSACTION");
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "启动事务失败";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 3. 删除own_table中的关联记录
  std::stringstream delete_own_sql;
  delete_own_sql << "DELETE FROM own_table WHERE username = '" << username
                 << "' AND virtual_file_path = '" << virtual_file_path << "';";

  res = mysql_query(mysql, delete_own_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "删除文件关联失败";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 4. 减少file_table中的引用计数
  std::stringstream update_file_sql;
  update_file_sql
      << "UPDATE file_table SET citation_count = citation_count - 1 "
      << "WHERE file_id = '" << file_id << "';";

  res = mysql_query(mysql, update_file_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "更新引用计数失败";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 5. 检查引用计数是否为0，如果是则删除文件记录和物理文件
  std::stringstream check_count_sql;
  check_count_sql << "SELECT citation_count, actual_file_path FROM file_table "
                  << "WHERE file_id = '" << file_id << "' FOR UPDATE;";

  res = mysql_query(mysql, check_count_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "检查引用计数失败";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  result = mysql_store_result(mysql);
  row = mysql_fetch_row(result);
  int citation_count = std::stoi(row[0]);
  std::string actual_file_path = row[1];
  mysql_free_result(result);

  if (citation_count <= 0) {
    // 删除file_table中的记录
    std::stringstream delete_file_sql;
    delete_file_sql << "DELETE FROM file_table WHERE file_id = '" << file_id
                    << "';";

    res = mysql_query(mysql, delete_file_sql.str().c_str());
    if (res) {
      mysql_query(mysql, "ROLLBACK");
      m_lock.unlock();
      response_json["status"] = "error";
      response_json["message"] = "删除文件记录失败";
      (*message_json)["server_content"] = response_json.dump(4);
      return;
    }

    // 提交事务
    res = mysql_query(mysql, "COMMIT");
    m_lock.unlock();

    if (res) {
      response_json["status"] = "error";
      response_json["message"] = "提交事务失败";
    } else {
      // 删除物理文件
      if (std::remove(actual_file_path.c_str()) == 0) {
        response_json["status"] = "success";
        response_json["message"] = "文件删除成功（包括物理文件）";
      } else {
        response_json["status"] = "warning";
        response_json["message"] = "文件记录已删除，但物理文件删除失败";
      }
    }
  } else {
    // 提交事务
    res = mysql_query(mysql, "COMMIT");
    m_lock.unlock();

    if (res) {
      response_json["status"] = "error";
      response_json["message"] = "提交事务失败";
    } else {
      response_json["status"] = "success";
      response_json["message"] = "文件关联已删除，物理文件仍被其他用户引用";
    }
  }

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<file_delete_way> file_delete_auto_register;

void file_rename_way::request_stratege(MYSQL *mysql,
                                       std::unique_ptr<json> &message_json) {
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string virtual_file_path = post_client["virtual_file_path"];
  std::string username = post_client["username"];
  std::string new_name_path = post_client["new_name_path"];

  // 1. 检查源文件是否存在
  std::stringstream check_src_sql;
  check_src_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '"
                << virtual_file_path << "';";

  m_lock.lock();
  int res = mysql_query(mysql, check_src_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int src_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (src_count == 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "source file not found";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 3. 执行重命名操作
  std::stringstream rename_file_sql;
  rename_file_sql << "UPDATE own_table SET virtual_file_path = '"
                  << new_name_path << "' "
                  << "WHERE username = '" << username
                  << "' AND virtual_file_path = '" << virtual_file_path << "';";

  res = mysql_query(mysql, rename_file_sql.str().c_str());
  m_lock.unlock();

  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "rename failed";
  } else {
    response_json["status"] = "success";
    response_json["message"] = "file renamed successfully";
  }

  (*message_json)["server_content"] = response_json.dump(4);
}

static auto_register<file_rename_way> file_rename_auto_register;

void file_move_way::request_stratege(MYSQL *mysql,
                                     std::unique_ptr<json> &message_json) {
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string virtual_file_path = post_client["virtual_file_path"];
  std::string username = post_client["username"];
  std::string new_file_path = post_client["new_file_path"];

  // 1. 检查源文件是否存在
  std::stringstream check_src_sql;
  check_src_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '"
                << virtual_file_path << "';";

  m_lock.lock();
  int res = mysql_query(mysql, check_src_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int src_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (src_count == 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "source file not found";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  std::stringstream move_file_sql;
  move_file_sql << "UPDATE own_table SET virtual_file_path = '" << new_file_path
                << "' "
                << "WHERE username = '" << username
                << "' AND virtual_file_path = '" << virtual_file_path << "';";

  res = mysql_query(mysql, move_file_sql.str().c_str());
  m_lock.unlock();

  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "move failed";
  } else {
    response_json["status"] = "success";
    response_json["message"] = "file moved successfully";
  }

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<file_move_way> file_move_auto_register;

void file_copy_way::request_stratege(MYSQL *mysql,
                                     std::unique_ptr<json> &message_json) {
  // 暂时不需要写
}
static auto_register<file_copy_way> file_copy_auto_register;

// 目录操作相关策略实现
void directory_create_way::request_stratege(
    MYSQL *mysql, std::unique_ptr<json> &message_json) {
  std::string user;
}
static auto_register<directory_create_way> directory_create_auto_register;

void directory_delete_way::request_stratege(
    MYSQL *mysql, std::unique_ptr<json> &message_json) {
  std::string user;
}
static auto_register<directory_delete_way> directory_delete_auto_register;

void directory_list_way::request_stratege(MYSQL *mysql,
                                          std::unique_ptr<json> &message_json) {
  std::string user;
}
static auto_register<directory_list_way> directory_list_auto_register;

void directory_rename_way::request_stratege(
    MYSQL *mysql, std::unique_ptr<json> &message_json) {
  std::string user;
}
static auto_register<directory_rename_way> directory_rename_auto_register;
