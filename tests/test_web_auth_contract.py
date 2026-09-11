from pathlib import Path


source = Path("src/webserver.c").read_text(encoding="utf-8")

assert 'POST /api/unlock ' in source
assert 'web_auth_unlock(&control_auth, pin, WEB_CONTROL_PIN' in source
assert 'get_rand_64()' in source
assert 'WEB_CONTROL_SESSION_MS' in source
assert source.count('if (!request_authorized(text)) return send_unauthorized') >= 7
assert 'POST /api/wifi ' in source and 'wifi_settings_add(ssid, password)' in source
assert 'POST /api/wifi-select ' in source and 'wifi_settings_select(ssid)' in source
assert 'POST /api/wifi-remove ' in source and 'wifi_settings_remove(ssid)' in source
assert 'POST /api/peltier-test?channel=' in source
assert 'POST /api/workshop-override ' in source
assert 'POST /api/presentation-demo?direction=' in source
assert 'GET /api/status ' in source
assert source.index('if (!request_authorized(text))', source.index('POST /api/start ')) < source.index('if (start_allowed())', source.index('POST /api/start '))
stop_handler = source[source.index('POST /api/stop '):source.index('POST /api/rgb-test ')]
assert 'request_authorized' not in stop_handler
assert 'server_config.stop();' in stop_handler
rgb_handler = source[source.index('POST /api/rgb-test '):source.index('static const char setpoint_prefix')]
assert 'if (!request_authorized(text))' in rgb_handler
assert 'server_config.rgb_test();' in rgb_handler
assert 'return strlen(request) >= header_length + request_content_length(request);' in source
assert 'server_config.start();' in source and 'safety_can_start(&effective)' in source

ui = Path("ui-v3/src/experience.js").read_text(encoding="utf-8")
assert 'fetchWithTimeout("/api/unlock"' in ui
assert 'command("start",true)' in ui
assert 'command("stop",false)' in ui
assert 'command("setpoint?value="+value.toFixed(1),true)' in ui
assert 'command("rgb-test",true)' in ui
assert 'peltier-test?channel=' in ui
assert 'command("workshop-override",true)' in ui
assert 'presentation-demo?direction=' in ui
assert 'fetchWithTimeout("/api/wifi"' in ui
assert 'rgbTestButton' in ui
assert 'body:token?"token="+encodeURIComponent(token):""' in ui
assert 'sessionStorage' not in ui and 'localStorage' not in ui

print("web auth contract test passed")
