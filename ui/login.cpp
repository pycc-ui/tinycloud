#include "login.h"
#include "information.h"
#include "process_http.h"
#include "src/json.hpp"
#include "ui_login.h"
#include "information.h"
using json = nlohmann::json;
login::login(QWidget *parent) : QWidget(parent), ui(new Ui::login) {
  ui->setupUi(this);
  const std::string ip = "192.168.228.128";
  const std::string port = "9006";
  const std::string download_path = "C:\\Users\\43720\\Downloads";
  const std::string remote_path = "/";

  information::getInstance().set_download_path(download_path);
  information::getInstance().set_ip(ip);
  information::getInstance().set_port(port);
  information::getInstance().set_current_remote_path(remote_path);
}

login::~login() { delete ui; }

void login::on_pushButton_clicked() {


  Process_http login_http;
  json text_json;
  QString user = ui->nameEdit->text();
  QString password = ui->pswEdit->text();
  text_json["username"] = user.toStdString();
  text_json["passwd"] = password.toStdString();
  if (login_http.http_send("POST", "/auth/login", "close", text_json)) {
    json rev_json;
    if (login_http.http_rev(rev_json)) {
      if (rev_json["body"]["status"] == "success") {
        information::getInstance().set_user(user.toStdString());
        information::getInstance().set_passwd(password.toStdString());
        emit loginSuccess();
        QWidget::close();
      } else {
        ui->promptLabel->setText("登录失败");
      }
    }
  } else {
    ui->promptLabel->setText("登录失败");
  }
}

void login::on_pushButton_2_clicked() {
  Process_http login_http;
  json text_json;
  QString user = ui->nameEdit->text();
  QString password = ui->pswEdit->text();
  text_json["username"] = user.toStdString();
  text_json["passwd"] = password.toStdString();

  if (login_http.http_send("POST", "/auth/register", "close", text_json)) {
    json rev_json;
    if (login_http.http_rev(rev_json)) {
      if (rev_json["body"]["status"] == "success") {

        ui->promptLabel->setText("注册成功,重新输入密码登录");
        ui->pswEdit->setText("");
      } else {
        ui->promptLabel->setText("注册失败");
      }
    } else {
      ui->promptLabel->setText("注册失败");
    }
  }
}
