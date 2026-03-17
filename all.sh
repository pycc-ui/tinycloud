#!/bin/bash

python3 ./test/clean_db.py

rm -rf ./root/*

rm -rf ./serverlog/*

pkill -9 -f server # 根据实际进程名调整

sleep 1

make

./server &
