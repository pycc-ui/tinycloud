#ifndef BASE64_H
#define BASE64_H
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <stdexcept>
#include <string>

std::string base64_encode(const std::string &input);

std::string base64_decode(const std::string &encoded_string);
#endif // !BASE64_H
