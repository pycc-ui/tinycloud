#ifndef INFORMATION_H
#define INFORMATION_H
#include <string>

class information {
public:
  static information &getInstance() {
    static information instance; // c++11内存安全
    return instance;
  }

  information(const information &) = delete;
  information &operator=(const information &) = delete;

  void set_ip(const std::string &ip) { m_ip = ip; }
  const std::string &get_ip() { return m_ip; }
  void set_port(const std::string &port) { m_port = port; }
  const std::string &get_port() { return m_port; }
  void set_user(const std::string &user) { m_user = user; }
  const std::string &get_user() { return m_user; }
  void set_passwd(const std::string &passwd) { m_passwd = passwd; }
  const std::string& get_passwd() { return m_passwd; }
  void set_current_remote_path(const std::string &current_path){m_current_remote_path = current_path; }
  const std::string& get_remote_path() {return m_current_remote_path;}
  void set_download_path(const std::string &download_path){m_download_path = download_path;}
  const std::string& get_download_path(){return m_download_path;}

private:
  information() = default;
  ~information() = default;
  std::string m_passwd;
  std::string m_user;
  std::string m_ip;
  std::string m_port;
  std::string m_current_remote_path;
  std::string m_download_path;
};

#endif // INFORMATION_H
