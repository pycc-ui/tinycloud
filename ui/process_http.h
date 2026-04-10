#ifndef PROCESS_HTTP_H
#define PROCESS_HTTP_H

#include "information.h"
#include "src/json.hpp"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <QString>

using json = nlohmann::json;

class Process_http : public QObject {
  Q_OBJECT
public:
  Process_http(QObject *parent = nullptr);
  ~Process_http();
  bool http_rev(json &rev_json);
  bool http_send(QString httpMethods, QString path, QString connection,
                 const json &send_json);

public:
  QString ip;        // 服务端ip地址
  QTcpSocket socket; // 套接字
  int port;          // 服务器端口
};

#endif // PROCESS_HTTP_H
