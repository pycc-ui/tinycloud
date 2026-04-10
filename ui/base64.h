#ifndef BASE64_H
#define BASE64_H

#include <QByteArray>
#include <QString>
#include <string>

// 保持原接口不变
std::string base64_encode(const std::string &input);
std::string base64_decode(const std::string &encoded_string);

#endif // BASE64_H
