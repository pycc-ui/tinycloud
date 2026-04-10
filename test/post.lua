wrk.method = "POST"
wrk.headers["Connection"] = "keep-alive" -- 强制关闭连接，避免keep-alive

-- 请求体内容（与日志中的一致，注意转义双引号）
wrk.body = [[
{
    "actual_file_path": "./root/ad/ac/adac40466fec488d91a41b1f02550348.bin",
    "block_begin": 0,
    "block_size": 16384,
    "downloading": true,
    "username": "111",
    "virtual_file_path": "/白底简历模板7551284678120558320.docx"
}
]]

wrk.headers["Content-Length"] = string.len(wrk.body)
