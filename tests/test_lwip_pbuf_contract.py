from pathlib import Path


source = Path("src/webserver.c").read_text(encoding="utf-8")

receive_start = source.index("static err_t receive(")
receive_end = source.index("static err_t accept_client(", receive_start)
receive = source[receive_start:receive_end]

assert receive.count("pbuf_free(packet);") == 3
assert "return result == ERR_ABRT ? ERR_ABRT : ERR_OK;" in receive
assert "tcp_abort(client);\n        return ERR_ABRT;" in receive

send_start = source.index("static err_t send_response(")
send_end = source.index("static void status_json(", send_start)
send = source[send_start:send_end]

assert "tcp_abort(client);\n    return ERR_ABRT;" in send
assert "if (error == ERR_OK) return ERR_OK;" in send
assert "return error;" not in send

print("lwIP pbuf ownership contract test passed")
