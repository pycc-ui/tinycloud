#include "base64.h"
// Base64编码函数 - 返回string
std::string base64_encode(const std::string &input) {
  if (input.empty()) {
    return "";
  }

  BIO *bmem, *b64;
  BUF_MEM *bptr;

  // 创建base64过滤器
  b64 = BIO_new(BIO_f_base64());
  if (!b64) {
    throw std::runtime_error("Failed to create BIO for base64 encoding");
  }

  // 设置标志：不添加换行符
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  // 创建内存BIO来存储结果
  bmem = BIO_new(BIO_s_mem());
  if (!bmem) {
    BIO_free(b64);
    throw std::runtime_error("Failed to create memory BIO");
  }

  // 连接base64过滤器到内存BIO
  b64 = BIO_push(b64, bmem);

  // 写入数据并刷新
  BIO_write(b64, input.data(), static_cast<int>(input.length()));
  BIO_flush(b64);

  // 获取内存指针
  BIO_get_mem_ptr(b64, &bptr);

  // 创建字符串结果
  std::string result(bptr->data, bptr->length);

  // 清理
  BIO_free_all(b64);

  return result;
}

// Base64解码函数 - 返回string
std::string base64_decode(const std::string &encoded_string) {
  if (encoded_string.empty()) {
    return "";
  }

  // 创建base64过滤器
  BIO *b64 = BIO_new(BIO_f_base64());
  if (!b64) {
    throw std::runtime_error("Failed to create BIO for base64 decoding");
  }

  // 设置标志：不处理换行符（与编码时一致）
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

  // 将编码字符串放入内存BIO
  BIO *mem = BIO_new_mem_buf(encoded_string.data(),
                             static_cast<int>(encoded_string.length()));
  if (!mem) {
    BIO_free(b64);
    throw std::runtime_error("Failed to create memory BIO for decoding");
  }

  // 连接base64过滤器到内存BIO
  mem = BIO_push(b64, mem);

  // 计算解码后最大可能的大小
  size_t max_decoded_length = (encoded_string.length() * 3) / 4;
  std::string decoded_string;
  decoded_string.resize(max_decoded_length + 1); // +1 for safety

  // 解码数据
  int decoded_length = BIO_read(mem, &decoded_string[0],
                                static_cast<int>(decoded_string.size()));

  if (decoded_length < 0) {
    BIO_free_all(mem);
    throw std::runtime_error("Failed to decode base64 data");
  }

  // 调整到实际解码大小
  decoded_string.resize(static_cast<size_t>(decoded_length));

  // 清理
  BIO_free_all(mem);

  return decoded_string;
}
