#include "sha256.h"

std::string calculateFileSHA256(const std::string &filepath) {
    QFile file(QString::fromStdString(filepath));

    if (!file.open(QIODevice::ReadOnly)) {
        std::ostringstream oss;
        oss << "无法打开文件: " << filepath;
        throw std::runtime_error(oss.str());
    }

    QCryptographicHash hash(QCryptographicHash::Sha256);

    // 分块读取文件，避免内存占用过大
    const qint64 bufferSize = 65536;
    char buffer[bufferSize];

    while (!file.atEnd()) {
        qint64 bytesRead = file.read(buffer, bufferSize);
        if (bytesRead > 0) {
            hash.addData(buffer, bytesRead);
        }
    }

    file.close();

    // 获取哈希结果
    QByteArray result = hash.result();

    // 转换为十六进制字符串
    std::ostringstream oss;
    oss << std::hex << std::setfill('0');

    for (int i = 0; i < result.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(result[i]);
        oss << std::setw(2) << static_cast<int>(c);
    }

    return oss.str();
}
