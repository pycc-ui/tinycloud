#include "./nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <sstream>
#include <string>

using json = nlohmann::json;
std::string calculateFileSHA256(const std::string &filepath);

std::string base64_encode(const std::string &input) {
  BIO *bmem = BIO_new(BIO_s_mem());
  BIO *b64 = BIO_new(BIO_f_base64());
  b64 = BIO_push(b64, bmem);
  BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
  BIO_write(b64, input.data(), input.length());
  BIO_flush(b64);

  BUF_MEM *bptr;
  BIO_get_mem_ptr(b64, &bptr);

  std::string result(bptr->data, bptr->length);

  BIO_free_all(b64);
  return result;
}

int main() {
  json content;
  content["username"] = "11";
  content["passwd"] = "11";
  std::string filepath("./test.txt");
  std::string sha256_num = calculateFileSHA256(filepath);

  std::ifstream file(filepath.c_str());

  file.seekg(0, std::ios::end);
  size_t size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::string file_content(size, ' ');
  file.read(&file_content[0], size);

  std::cout << "sha256_num:";
  std::cout << sha256_num << std::endl;
  content["document_content"] = file_content;
  content["sha256_num"] = sha256_num.c_str();
  content["virtual_file_path"] = "/test.txt";
  content["file_size"] = std::to_string(size);

  file.close();
  std::string content_string = content.dump();
  std::stringstream http_request;

  http_request << "POST /file/upload HTTP/1.1\r\n"
               << "Host: localhost\r\n"
               << "Content-Length: " << content_string.size() << "\r\n"
               << "Connection: keep-alive\r\n\r\n"
               << content_string << " ";

  std::ofstream tempflie("/tmp/http_request.txt");

  http_request << "POST /file/upload HTTP/1.1\r\n"
               << "Host: localhost\r\n"
               << "Content-Length: " << content_string.size() << "\r\n"
               << "Connection: close\r\n\r\n"
               << content_string << " ";
  tempflie << http_request.str();

  tempflie.close();

  std::string command = "cat /tmp/http_request.txt | nc  -w 200 localhost 9006";

  int result = system(command.c_str());

  system("rm /tmp/http_request.txt");

  return 0;
}

std::string calculateFileSHA256(const std::string &filepath) {
  std::ifstream file(filepath, std::ios::binary);
  EVP_MD_CTX *ctx = EVP_MD_CTX_new();
  EVP_DigestInit_ex(ctx, EVP_sha256(), NULL);

  const size_t BUFFER_SIZE = 65536;
  char buffer[BUFFER_SIZE];

  while (file.read(buffer, BUFFER_SIZE) || file.gcount() > 0) {
    EVP_DigestUpdate(ctx, buffer, file.gcount());
  }

  unsigned char hash[EVP_MAX_MD_SIZE];
  unsigned int hash_len = 0;
  EVP_DigestFinal_ex(ctx, hash, &hash_len);
  EVP_MD_CTX_free(ctx);

  // 直接使用stringstream，但要确保正确
  std::ostringstream oss;
  oss << std::hex << std::setfill('0');
  for (unsigned int i = 0; i < hash_len; i++) {
    oss << std::setw(2) << (int)(hash[i] & 0xFF); // 关键：使用 & 0xFF
  }

  return oss.str();
}
