#include "../base64/base64.h"
#include "../nlohmann/json.hpp"
#include <fstream>
#include <iostream>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <sstream>
#include <string>

using json = nlohmann::json;

int main() {
  std::string content_string = R"({
    "appanding": false,
    "file_size": "7023470",
    "passwd": "111",
    "sha256_num": "fb80442754866415d7ee274fe20522207d0754097ff3a6b1b6b0bb355cd5f589",
    "username": "111",
    "virtual_file_path": "/numbers.txt"
})";
  json a = json::parse(content_string);
  content_string = a.dump(2);
  std::stringstream http_request;

  http_request << "POST /file/upload HTTP/1.1\r\n"
               << "Host: localhost:9006\r\n"
               << "Content-Length: " << content_string.size() << "\r\n"
               << "Connection: keep-alive\r\n\r\n"
               << content_string << " ";

  std::ofstream tempflie("/tmp/http_request.txt");

  std::cout << http_request.str();
  tempflie << http_request.str();
  fflush(stdout);

  tempflie.close();

  std::string command =
      "cat /tmp/http_request.txt | nc  -w 2000 localhost 9006";

  int result = system(command.c_str());

  system("rm /tmp/http_request.txt");

  return 0;
}
