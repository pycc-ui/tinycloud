#ifndef REQUEST_WAY_H
#define REQUEST_WAY_H

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>

#include "../CGImysql/sql_connection_pool.h"
#include "../http/JsonPool.h"
#include "../lock/locker.h"
#include "../log/log.h"
#include "../nlohmann/json.hpp"

using json = nlohmann::json;

// 策略接口类
class request_way {
  friend class way_manager;

public:
  request_way() = default;
  virtual ~request_way() = default;

protected:
  static locker m_lock;

private:
  virtual void request_stratege(MYSQL *mysql,
                                JsonPool::PtrType &message_json) = 0;

  virtual std::string get_name() = 0;
};

// 策略使用接口
class way_manager {
  template <typename T> friend class auto_register;
  int m_close_log;

public:
  way_manager(int close_log) { m_close_log = close_log; }
  ~way_manager() = default;
  void do_way(MYSQL *mysql, JsonPool::PtrType &message_json) {
    auto &creators = get_creator();
    auto it = creators.find((*message_json)["url"]);
    if (it != creators.end() && it->second != nullptr) {
      try {
        it->second->request_stratege(mysql, message_json);
      } catch (const std::exception &e) {
        // 获取异常信息
        LOG_ERROR("%s : %s", typeid(e).name(), e.what());
        json response_json;
        response_json["status"] = "error";
        response_json["message"] = "request stratege exec error";
        (*message_json)["server_content"] = response_json.dump(4);
      }
    } else {
      json response_json;
      response_json["status"] = "error";
      response_json["message"] = "No endpoints";
      (*message_json)["server_content"] = response_json.dump(4);
    }
  }

private:
  static std::unordered_map<std::string, std::unique_ptr<request_way>> &
  get_creator() {
    static std::unordered_map<std::string, std::unique_ptr<request_way>>
        creator_ways;
    return creator_ways;
  }

  static void register_creator(std::unique_ptr<request_way> way) {
    if (way != nullptr) {
      get_creator()[std::move(way->get_name())] = std::move(way);
    }
  }
};

// 策略注册接口
template <typename T> class auto_register {
public:
  auto_register() { way_manager::register_creator(std::make_unique<T>()); }
};

// 登录策略
class login_way : public request_way {
public:
  login_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/auth/login"; }
};

// 注册策略
class register_way : public request_way {
public:
  register_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/auth/register"; }
};

// 文件下载策略
class file_download_way : public request_way {
public:
  file_download_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/download"; }
};

// 文件上传策略
class file_upload_way : public request_way {
public:
  file_upload_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/upload"; }
};

class file_delete_way : public request_way {
public:
  file_delete_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/delete"; }
};

class file_rename_way : public request_way {
public:
  file_rename_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/rename"; }
};

class file_move_way : public request_way {
public:
  file_move_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/move"; }
};

class file_copy_way : public request_way {
public:
  file_copy_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/file/copy"; }
};

class directory_create_way : public request_way {
public:
  directory_create_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/directory/create"; }
};

class directory_delete_way : public request_way {
public:
  directory_delete_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/directory/delete"; }
};

class directory_list_way : public request_way {
public:
  directory_list_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/directory/list"; }
};

class directory_rename_way : public request_way {
public:
  directory_rename_way() = default;
  void request_stratege(MYSQL *mysql, JsonPool::PtrType &message_json) override;

  std::string get_name() override { return "/directory/rename"; }
};

#endif // !REQUEST_WAY_H
