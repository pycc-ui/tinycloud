#ifndef SHA256_H
#define SHA256_H

#include <QCryptographicHash>
#include <QFile>
#include <QDebug>

std::string calculateFileSHA256(const std::string &filepath);

#endif // SHA256_H
