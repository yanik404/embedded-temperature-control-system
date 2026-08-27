from pathlib import Path


source = Path("src/webserver.c").read_text(encoding="utf-8")

assert 'POST /api/unlock ' in source
assert 'web_auth_unlock(&control_auth, pin, WEB_CONTROL_PIN' in source
assert 'get_rand_64()' in source
assert 'WEB_CONTROL_SESSION_MS' in source
assert source.count('if (!request_authorized(text)) return send_unauthorized') == 3
assert 'GET /api/status ' in source
assert source.index('if (!request_authorized(text))', source.index('POST /api/start ')) < source.index('if (start_allowed())', source.index('POST /api/start '))
stop_handler = source[source.index('POST /api/stop '):source.index('POST /api/rgb-test ')]
assert 'request_authorized' not in stop_handler
assert 'server_config.stop();' in stop_handler
rgb_handler = source[source.index('POST /api/rgb-test '):source.index('static const char setpoint_prefix')]
assert 'if (!request_authorized(text))' in rgb_handler
assert 'server_config.rgb_test();' in rgb_handler
assert 'return strlen(request) >= header_length + request_content_length(request);' in source
assert 'server_config.start();' in source and 'safety_can_start(s)' in source

ui = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
assert 'fetchWithTimeout("/api/unlock"' in ui
assert 'command("start",true)' in ui
assert 'command("stop",false)' in ui
assert 'command("setpoint?value="+value.toFixed(1),true)' in ui
assert 'command("rgb-test",true)' in ui
assert 'rgbTestButton' in ui
assert 'body:token?"token="+encodeURIComponent(token):""' in ui
assert 'sessionStorage' not in ui and 'localStorage' not in ui

print("web auth contract test passed")
