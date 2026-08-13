from pathlib import Path


source = Path("src/webserver.c").read_text(encoding="utf-8")

assert 'POST /api/unlock ' in source
assert 'web_auth_unlock(&control_auth, pin, WEB_CONTROL_PIN' in source
assert 'get_rand_64()' in source
assert 'WEB_CONTROL_SESSION_MS' in source
assert source.count('if (!request_authorized(text)) return send_unauthorized') == 3
assert 'GET /api/status ' in source
assert source.index('if (!request_authorized(text))', source.index('POST /api/start ')) < source.index('if (start_allowed())', source.index('POST /api/start '))
assert 'return strlen(request) >= header_length + request_content_length(request);' in source
assert 'server_config.start();' in source and 'safety_can_start(s)' in source

ui = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
assert 'fetchWithTimeout("/api/unlock"' in ui
assert 'body:"token="+encodeURIComponent(runtime.token)' in ui
assert 'sessionStorage' not in ui and 'localStorage' not in ui

print("web auth contract test passed")
