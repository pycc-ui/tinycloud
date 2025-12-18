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

  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  string new_dir_path = post_client["new_dir_path"];
  string username = post_client["username"];

  // 1. 检查目录是否已存在
  std::stringstream check_sql;
  check_sql << "SELECT COUNT(*) FROM own_table WHERE username = '" << username
            << "' AND virtual_file_path = '" << new_dir_path << "';";

  m_lock.lock();
  int res = mysql_query(mysql, check_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int count = std::stoi(row[0]);
  mysql_free_result(result);

  if (count > 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "directory already exists";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 2. 插入目录记录
  // 目录使用特殊的file_id "directory"，is_dir设置为1
  std::stringstream insert_dir_sql;
  insert_dir_sql
      << "INSERT INTO own_table(file_id, virtual_file_path, username, is_dir) "
      << "VALUES('directory', '" << new_dir_path << "', '" << username
      << "', 1);";

  res = mysql_query(mysql, insert_dir_sql.str().c_str());
  m_lock.unlock();

  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "directory creation failed";
  } else {
    response_json["status"] = "success";
    response_json["message"] = "directory created successfully";
  }

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<directory_create_way> directory_create_auto_register;

void directory_delete_way::request_stratege(
    MYSQL *mysql, std::unique_ptr<json> &message_json) {

  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string virtual_dir_path = post_client["virtual_dir_path"];
  std::string username = post_client["username"];

  // 1. 检查目录是否存在且确实是目录
  std::stringstream check_dir_sql;
  check_dir_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '" << virtual_dir_path
                << "' AND is_dir = 1;";

  m_lock.lock();
  int res = mysql_query(mysql, check_dir_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int dir_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (dir_count == 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "directory not found or not a directory";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 2. 检查目录是否为空（不包含子文件或子目录）
  std::stringstream check_empty_sql;
  check_empty_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                  << username << "' AND virtual_file_path LIKE '"
                  << virtual_dir_path << "/%';";

  res = mysql_query(mysql, check_empty_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  result = mysql_store_result(mysql);
  row = mysql_fetch_row(result);
  int child_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (child_count > 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "directory is not empty, cannot delete";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 3. 删除目录记录
  std::stringstream delete_dir_sql;
  delete_dir_sql << "DELETE FROM own_table WHERE username = '" << username
                 << "' AND virtual_file_path = '" << virtual_dir_path
                 << "' AND is_dir = 1;";

  res = mysql_query(mysql, delete_dir_sql.str().c_str());
  m_lock.unlock();

  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "directory deletion failed";
  } else {
    response_json["status"] = "success";
    response_json["message"] = "directory deleted successfully";
  }

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<directory_delete_way> directory_delete_auto_register;

void directory_list_way::request_stratege(MYSQL *mysql,
                                          std::unique_ptr<json> &message_json) {
  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string virtual_dir_path = post_client["virtual_dir_path"];
  std::string username = post_client["username"];

  // 1. 检查目录是否存在且确实是目录
  std::stringstream check_dir_sql;
  check_dir_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '" << virtual_dir_path
                << "' AND is_dir = 1;";

  m_lock.lock();
  int res = mysql_query(mysql, check_dir_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int dir_count = std::stoi(row[0]);
  mysql_free_result(result);

  // 如果是根目录，不检查存在性（根目录可能不存在记录）
  bool is_root_dir = (virtual_dir_path == "/" || virtual_dir_path.empty());

  if (!is_root_dir && dir_count == 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "directory not found or not a directory";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 2. 查询目录下的直接子项
  // 使用 LIKE 查询匹配下一级，不查询更深的层级
  std::stringstream list_sql;

  if (is_root_dir) {
    // 根目录：查询所有不包含斜杠的直接子项，或者以/开头后没有斜杠的
    // 例如: /file1, /dir1
    list_sql
        << "SELECT virtual_file_path, is_dir FROM own_table WHERE username = '"
        << username << "' AND virtual_file_path REGEXP '^/[^/]+$';";
  } else {
    // 普通目录：查询以目录路径+斜杠开头，且后面没有斜杠的直接子项
    // 例如: /dir1/file1, /dir1/subdir1
    std::string pattern = virtual_dir_path + "/[^/]+$";
    list_sql
        << "SELECT virtual_file_path, is_dir FROM own_table WHERE username = '"
        << username << "' AND virtual_file_path REGEXP '" << pattern << "';";
  }

  res = mysql_query(mysql, list_sql.str().c_str());
  m_lock.unlock();

  if (res) {
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  result = mysql_store_result(mysql);
  if (!result) {
    response_json["status"] = "error";
    response_json["message"] = "no result from query";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 3. 构建返回的目录列表
  json dir_list_json = json::array();
  int num_rows = mysql_num_rows(result);

  if (num_rows > 0) {
    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
      std::string full_path = row[0];
      int is_dir = std::stoi(row[1]);

      // 提取文件名或目录名（最后一部分）
      size_t last_slash = full_path.find_last_of('/');
      std::string name;
      if (last_slash != std::string::npos) {
        name = full_path.substr(last_slash + 1);
      } else {
        name = full_path; // 根目录下的情况
      }

      // 构建项信息
      json item;
      item["name"] = name;
      item["full_path"] = full_path;
      item["is_dir"] = is_dir;

      // 如果是文件，还需要查询文件大小等信息
      if (is_dir == 0) {
        // 查询文件详细信息
        std::stringstream file_info_sql;
        file_info_sql << "SELECT f.file_size, f.sha256_num FROM file_table f "
                      << "JOIN own_table o ON f.file_id = o.file_id "
                      << "WHERE o.username = '" << username
                      << "' AND o.virtual_file_path = '" << full_path << "';";

        m_lock.lock();
        int info_res = mysql_query(mysql, file_info_sql.str().c_str());
        if (!info_res) {
          MYSQL_RES *info_result = mysql_store_result(mysql);
          if (info_result && mysql_num_rows(info_result) > 0) {
            MYSQL_ROW info_row = mysql_fetch_row(info_result);
            item["file_size"] = info_row[0];
            item["sha256"] = info_row[1];
          }
          if (info_result)
            mysql_free_result(info_result);
        }
        m_lock.unlock();
      }

      dir_list_json.push_back(item);
    }
  }

  mysql_free_result(result);

  // 4. 返回结果
  response_json["status"] = "success";
  response_json["message"] = "directory list retrieved";
  response_json["dir_list"] = dir_list_json;

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<directory_list_way> directory_list_auto_register;

void directory_rename_way::request_stratege(
    MYSQL *mysql, std::unique_ptr<json> &message_json) {

  string client_content = (*message_json)["client_content"];
  json post_client = json::parse(client_content);
  message_json->erase("client_content");
  json response_json;

  std::string old_dir_path = post_client["old_dir_path"];
  std::string username = post_client["username"];
  std::string new_dir_path = post_client["new_dir_path"];

  // 1. 检查旧目录是否存在且确实是目录
  std::stringstream check_old_sql;
  check_old_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '" << old_dir_path
                << "' AND is_dir = 1;";

  m_lock.lock();
  int res = mysql_query(mysql, check_old_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  MYSQL_RES *result = mysql_store_result(mysql);
  MYSQL_ROW row = mysql_fetch_row(result);
  int old_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (old_count == 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "old directory not found or not a directory";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 2. 检查新目录路径是否已存在
  std::stringstream check_new_sql;
  check_new_sql << "SELECT COUNT(*) FROM own_table WHERE username = '"
                << username << "' AND virtual_file_path = '" << new_dir_path
                << "';";

  res = mysql_query(mysql, check_new_sql.str().c_str());
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "sql query error";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  result = mysql_store_result(mysql);
  row = mysql_fetch_row(result);
  int new_count = std::stoi(row[0]);
  mysql_free_result(result);

  if (new_count > 0) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "new directory path already exists";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 3. 开始事务
  res = mysql_query(mysql, "START TRANSACTION");
  if (res) {
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "start transaction failed";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 4. 重命名目录本身
  std::stringstream rename_dir_sql;
  rename_dir_sql << "UPDATE own_table SET virtual_file_path = '" << new_dir_path
                 << "' WHERE username = '" << username
                 << "' AND virtual_file_path = '" << old_dir_path
                 << "' AND is_dir = 1;";

  res = mysql_query(mysql, rename_dir_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "rename directory failed";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  // 5. 重命名目录下的所有子文件和子目录
  // 注意：这里需要使用 MySQL 的 REPLACE 函数来替换路径前缀
  // 但 MySQL 的 UPDATE 不支持直接使用 REPLACE 更新自己，所以需要先查找再更新
  // 这里使用一个简单的策略：先查询出所有子路径，然后逐条更新

  // 查询所有子路径
  std::stringstream get_children_sql;
  get_children_sql
      << "SELECT virtual_file_path FROM own_table WHERE username = '"
      << username << "' AND virtual_file_path LIKE '" << old_dir_path << "/%';";

  res = mysql_query(mysql, get_children_sql.str().c_str());
  if (res) {
    mysql_query(mysql, "ROLLBACK");
    m_lock.unlock();
    response_json["status"] = "error";
    response_json["message"] = "get children paths failed";
    (*message_json)["server_content"] = response_json.dump(4);
    return;
  }

  result = mysql_store_result(mysql);
  if (result) {
    MYSQL_ROW child_row;
    while ((child_row = mysql_fetch_row(result))) {
      std::string old_child_path = child_row[0];
      // 计算新路径：将 old_dir_path 前缀替换为 new_dir_path
      std::string new_child_path =
          new_dir_path + old_child_path.substr(old_dir_path.length());

      std::stringstream update_child_sql;
      update_child_sql << "UPDATE own_table SET virtual_file_path = '"
                       << new_child_path << "' WHERE username = '" << username
                       << "' AND virtual_file_path = '" << old_child_path
                       << "';";

      res = mysql_query(mysql, update_child_sql.str().c_str());
      if (res) {
        mysql_free_result(result);
        mysql_query(mysql, "ROLLBACK");
        m_lock.unlock();
        response_json["status"] = "error";
        response_json["message"] = "update child path failed";
        (*message_json)["server_content"] = response_json.dump(4);
        return;
      }
    }
    mysql_free_result(result);
  }

  // 6. 提交事务
  res = mysql_query(mysql, "COMMIT");
  m_lock.unlock();

  if (res) {
    mysql_query(mysql, "ROLLBACK");
    response_json["status"] = "error";
    response_json["message"] = "commit transaction failed";
  } else {
    response_json["status"] = "success";
    response_json["message"] = "directory renamed successfully";
  }

  (*message_json)["server_content"] = response_json.dump(4);
}
static auto_register<directory_rename_way> directory_rename_auto_register;
