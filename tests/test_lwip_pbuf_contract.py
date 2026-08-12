from pathlib import Path


source = Path("src/webserver.c").read_text(encoding="utf-8")

receive_start = source.index("static err_t receive(")
receive_end = source.index("static err_t accept_client(", receive_start)
receive = source[receive_start:receive_end]

assert receive.count("pbuf_free(packet);") == 5
assert "return result == ERR_ABRT ? ERR_ABRT : ERR_OK;" in receive
assert "return abort_http_client(" in receive

send_start = source.index("static err_t send_response_data(")
send_end = source.index("static void status_json(", send_start)
send = source[send_start:send_end]

assert "tcp_sndbuf(client)" in send
assert "tcp_sent(client, response_sent);" in send
assert "tcp_poll(client, response_poll, 2u);" in send
assert "if (error == ERR_MEM)" in send
assert "tcp_abort(client);" in send
assert "context->response_acked < total" in send

print("lwIP pbuf ownership contract test passed")
