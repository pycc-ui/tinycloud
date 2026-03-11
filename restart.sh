#!/bin/bash
# 先杀死旧进程（如果有）
pkill -f server   # 根据实际进程名调整

# 等待进程完全退出（可选）
sleep 1

# 重新编译（如果需要）
make

# 启动新进程
./server &
