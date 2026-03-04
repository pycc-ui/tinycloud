"""
clean_db.py - 清空 MySQL 数据库中的指定表（处理外键约束）
配置信息直接写在代码中（仅用于开发测试）
依赖：mysql-connector-python
"""

import mysql.connector
from mysql.connector import Error

# 数据库连接配置（直接硬编码）
DB_CONFIG = {
    "host": "localhost",
    "user": "ruto",
    "password": "crutos",
    "database": "mybase",
}

# 要清空的表列表
TABLES = ["file_table", "own_table", "user_table"]


def truncate_tables(cursor, tables):
    # 禁用外键检查
    cursor.execute("SET FOREIGN_KEY_CHECKS = 0")
    for table in tables:
        print(f"Truncating {table}...")
        cursor.execute(f"TRUNCATE TABLE {table}")
        print(f"✅ {table} truncated")
    # 重新启用外键检查
    cursor.execute("SET FOREIGN_KEY_CHECKS = 1")


def main():
    conn = None
    cursor = None
    try:
        conn = mysql.connector.connect(**DB_CONFIG)
        cursor = conn.cursor()
        truncate_tables(cursor, TABLES)
        conn.commit()  # TRUNCATE 是 DDL，会自动提交，但此处保留 commit 无妨
        print("🎉 All tables truncated successfully.")
    except Error as e:
        print(f"❌ Database error: {e}")
        exit(1)
    finally:
        if cursor is not None:
            cursor.close()
        if conn is not None and conn.is_connected():
            conn.close()


if __name__ == "__main__":
    main()
