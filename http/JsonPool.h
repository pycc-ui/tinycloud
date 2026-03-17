// JsonPool.h
#pragma once
#include "../nlohmann/json.hpp"
#include <memory>
#include <mutex>
#include <vector>

using json = nlohmann::json;

class JsonPool {
public:
  using DeleterType = std::function<void(json *)>;
  using PtrType = std::unique_ptr<json, DeleterType>;

  JsonPool(const JsonPool &) = delete;
  JsonPool &operator=(const JsonPool &) = delete;

  static JsonPool &instance() {
    static JsonPool inst; // 静态局部变量，C++11 保证线程安全的一次构造
    return inst;
  }

  void init(size_t pool_size = 32) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (initialized_)
      return; // 防止重复初始化
    for (size_t i = 0; i < pool_size; ++i) {
      pool_.push_back(std::unique_ptr<json>(new json()));
    }
    initialized_ = true;
  }

  std::unique_ptr<json, DeleterType> get() {
    std::lock_guard<std::mutex> lock(mutex_);
    ensure_initialized(); // 确保池已初始化（如果用户忘记调用 init）

    if (pool_.empty()) {
      for (int i = 0; i < 8; ++i) {
        pool_.push_back(std::unique_ptr<json>(new json()));
      }
    }

    // 取出对象并绑定自定义删除器
    std::unique_ptr<json, DeleterType> ptr(
        pool_.back().release(), [this](json *p) {
          p->clear();
          std::lock_guard<std::mutex> lock(mutex_);
          pool_.push_back(std::unique_ptr<json>(p));
        });

    pool_.pop_back();
    return ptr;
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return pool_.size();
  }

private:
  JsonPool() = default; // 私有构造函数
  ~JsonPool() = default;

  void ensure_initialized() {
    if (!initialized_) {
      // 默认初始化 64 个对象
      for (size_t i = 0; i < 64; ++i) {
        pool_.push_back(std::unique_ptr<json>(new json()));
      }
      initialized_ = true;
    }
  }

  std::vector<std::unique_ptr<json>> pool_;
  mutable std::mutex mutex_;
  bool initialized_ = false; // 是否已初始化（用于用户自定义 init）
};
