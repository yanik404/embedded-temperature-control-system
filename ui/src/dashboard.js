(function () {
  "use strict";

  const PREVIEW = __PREVIEW_MODE__;
  const SAMPLE_INTERVAL = 500;
  const MAX_HISTORY_SECONDS = 30 * 60;
  const stateDescriptions = {
    AUS: "Leistungspfad sicher deaktiviert",
    BEREIT: "Alle Bedingungen für einen bewussten Start werden geprüft",
    AUFHEIZEN: "PI-Regler führt thermische Energie zu",
    HALTEN: "Solltemperatur erreicht · Energie wird nachgeführt",
    FEHLER: "Safety Lock aktiv · Heizleistung ist gesperrt"
  };
  const faultDescriptions = {
    "KEIN FEHLER": "Keine aktive Störung",
    TEMPERATURSENSOR: "Temperatursignal fehlt oder ist unplausibel",
    UEBERTEMPERATUR: "Maximale sichere Temperatur überschritten",
    LUEFTER: "Kein plausibles Tachosignal bei aktiver Heizleistung",
    UEBERSTROM: "Stromgrenze eines Heizkanals überschritten",
    STROMSENSOR: "Strommessung nicht plausibel",
    BECHER_ENTFERNT: "Becher während des Heizvorgangs entfernt",
    VERSORGUNG: "Leistungsversorgung nicht freigegeben"
  };
  const safetyDefinition = [
    ["temperature", "TEMPERATUR"],
    ["current", "STROM"],
    ["fan", "LÜFTER"],
    ["cup", "BECHER"],
    ["power", "VERSORGUNG"],
    ["network", "NETZWERK"],
    ["server", "HTTP"]
  ];
  const history = [];
  const events = [];
  const runtime = {
    online: false,
    initial: true,
    busy: false,
    previous: null,
    current: null,
    lastUpdate: 0,
    range: 60,
    lowMotion: matchMedia("(prefers-reduced-motion: reduce)").matches,
    heatingStarted: 0,
    holdingStarted: 0,
    maxError: 0,
    initialTemperature: null,
    drawQueued: false
  };

  const byId = id => document.getElementById(id);
  const all = selector => Array.from(document.querySelectorAll(selector));
  const finite = (value, fallback = 0) => Number.isFinite(Number(value)) ? Number(value) : fallback;
  const truth = (value, fallback = false) => typeof value === "boolean" ? value : fallback;
  const clamp = (value, minimum, maximum) => Math.max(minimum, Math.min(maximum, value));
  const fixed = (value, digits = 1) => Number.isFinite(Number(value)) ? Number(value).toFixed(digits).replace(".", ",") : "—";
  const setText = (id, value) => { const node = byId(id); if (node) node.textContent = value; };
  const available = (id, yes) => { const node = byId(id); if (node) node.classList.toggle("unavailable", !yes); };
  const statusWord = value => value ? "OK" : "OFFEN";

  function normalize(source) {
    source = source || {};
    const temperature = finite(source.temperature, NaN);
    const setpoint = finite(source.setpoint, 45);
    return {
      state: ["AUS", "BEREIT", "AUFHEIZEN", "HALTEN", "FEHLER"].includes(source.state) ? source.state : "AUS",
      fault: source.fault || "KEIN FEHLER",
      fault_description: source.fault_description || faultDescriptions[source.fault] || "Unbekannter Systemfehler",
      temperature,
      temperature1: finite(source.temperature1, NaN),
      temperature2: finite(source.temperature2, NaN),
      setpoint,
      error: Number.isFinite(Number(source.error)) ? Number(source.error) : setpoint - temperature,
      power: clamp(finite(source.power), 0, 100),
      fan_rpm: Math.max(0, finite(source.fan_rpm)),
      fan_percent: clamp(finite(source.fan_percent), 0, 100),
      current1: Math.max(0, finite(source.current1)),
      current2: Math.max(0, finite(source.current2)),
      light_level: clamp(finite(source.light_level), 0, 1),
      sensor_ok: truth(source.sensor_ok),
      temp1_ok: truth(source.temp1_ok),
      temp2_ok: truth(source.temp2_ok),
      current_ok: truth(source.current_ok),
      current1_ok: truth(source.current1_ok),
      current2_ok: truth(source.current2_ok),
      tla2024_ok: truth(source.tla2024_ok),
      light_ok: truth(source.light_ok),
      display_initialized: truth(source.display_initialized),
      leds_initialized: truth(source.leds_initialized),
      cup: truth(source.cup),
      power_good: truth(source.power_good),
      wifi: truth(source.wifi),
      webserver_ready: truth(source.webserver_ready),
      wifi_ssid: String(source.wifi_ssid || "—"),
      wifi_ip: String(source.wifi_ip || "—"),
      night: truth(source.night),
      start_allowed: truth(source.start_allowed),
      start_block_reason: String(source.start_block_reason || "Startfreigabe nicht bestätigt"),
      kp: finite(source.kp),
      ki: finite(source.ki),
      p_term: finite(source.p_term),
      i_term: finite(source.i_term),
      output_limited: truth(source.output_limited),
      anti_windup: truth(source.anti_windup),
      control_period_ms: Math.max(0, finite(source.control_period_ms)),
      max_safe_temperature: finite(source.max_safe_temperature, 65),
      uptime_ms: Math.max(0, finite(source.uptime_ms))
    };
  }

  function formatDuration(milliseconds) {
    const seconds = Math.max(0, Math.floor(milliseconds / 1000));
    const hours = Math.floor(seconds / 3600);
    const minutes = Math.floor(seconds % 3600 / 60);
    const remainder = seconds % 60;
    return hours ? `${hours} h ${String(minutes).padStart(2, "0")} min` : `${minutes}:${String(remainder).padStart(2, "0")} min`;
  }

  function setOnline(online, message) {
    if (runtime.online === online && !runtime.initial) return;
    runtime.online = online;
    document.body.dataset.live = String(online);
    setText("connectionLabel", online ? "LIVE · PICO W" : "SIGNAL UNTERBROCHEN");
    if (!online) {
      setText("networkIdentity", message || "Pico nicht erreichbar");
      setText("lastUpdate", runtime.lastUpdate ? new Date(runtime.lastUpdate).toLocaleTimeString("de-DE") : "—");
      addEvent("Verbindung zum Pico unterbrochen", "bad");
    } else if (!runtime.initial) {
      addEvent("Live-Verbindung wiederhergestellt", "good");
    }
  }

  function addEvent(message, tone = "info") {
    const latest = events[0];
    if (latest && latest.message === message && Date.now() - latest.time < 1500) return;
    events.unshift({ message, tone, time: Date.now() });
    events.splice(30);
    const timeline = byId("eventTimeline");
    if (!timeline) return;
    timeline.replaceChildren(...events.slice(0, 10).map(event => {
      const item = document.createElement("li");
      item.className = event.tone;
      const time = document.createElement("time");
      time.dateTime = new Date(event.time).toISOString();
      time.textContent = new Date(event.time).toLocaleTimeString("de-DE", { hour: "2-digit", minute: "2-digit", second: "2-digit" });
      const point = document.createElement("i");
      const text = document.createElement("span");
      text.textContent = event.message;
      item.append(time, point, text);
      return item;
    }));
  }

  function deriveEvents(status) {
    const old = runtime.previous;
    if (!old) {
      addEvent(PREVIEW ? "Lokale thermische Simulation gestartet" : "Dashboard mit Pico synchronisiert", "good");
      addEvent(`Systemzustand ${status.state}`);
      runtime.previous = { ...status };
      return;
    }
    if (old.state !== status.state) {
      const copy = { AUS: "System sicher ausgeschaltet", BEREIT: "System bereit", AUFHEIZEN: "Heizprozess bewusst gestartet", HALTEN: "Sollwert erreicht · Halteregelung aktiv", FEHLER: `Safety Lock: ${status.fault_description}` };
      addEvent(copy[status.state], status.state === "FEHLER" ? "bad" : status.state === "HALTEN" ? "good" : "info");
    }
    if (Math.abs(old.setpoint - status.setpoint) > .01) addEvent(`Sollwert auf ${fixed(status.setpoint)} °C geändert`);
    if (old.wifi !== status.wifi) addEvent(status.wifi ? `WLAN verbunden · ${status.wifi_ssid}` : "WLAN getrennt", status.wifi ? "good" : "bad");
    if (old.power_good !== status.power_good) addEvent(status.power_good ? "Leistungsversorgung freigegeben" : "Leistungsversorgung nicht verfügbar", status.power_good ? "good" : "warn");
    if (old.fault !== status.fault && status.fault !== "KEIN FEHLER") addEvent(status.fault_description, "bad");
    runtime.previous = { ...status };
  }

  function safetyStates(status) {
    const active = status.state === "AUFHEIZEN" || status.state === "HALTEN";
    return {
      temperature: status.sensor_ok && status.temp1_ok && status.temperature < status.max_safe_temperature,
      current: status.current_ok && status.current1_ok && status.current2_ok,
      fan: !active || status.fan_rpm > 100,
      cup: status.cup,
      power: status.power_good,
      network: status.wifi,
      server: status.webserver_ready
    };
  }

  function renderSafety(status) {
    const states = safetyStates(status);
    const nodes = safetyDefinition.map(([key, label]) => {
      const node = document.createElement("span");
      node.className = `interlock-node ${states[key] ? "ok" : "bad"}`;
      node.innerHTML = `<i></i><strong>${label}</strong><small>${states[key] ? "FREI" : "GESPERRT"}</small>`;
      return node;
    });
    byId("safetyInterlock").replaceChildren(...nodes);
  }

  function componentData(status) {
    const inactive = status.power <= 0;
    const item = (name, group, ok, state, detail) => ({ name, group, ok, state, detail });
    return [
      item("Raspberry Pi Pico W", "SYSTEM", runtime.online, runtime.online ? "ONLINE" : "OFFLINE", `Uptime ${formatDuration(status.uptime_ms)}`),
      item("OLED SSD1306", "INTERFACE", status.display_initialized, status.display_initialized ? "OK" : "NICHT VERFÜGBAR", "128 × 64 · I²C"),
      item("TMP36 Sensor 1", "SENSORIK", status.temp1_ok, status.temp1_ok ? "OK" : "FEHLER", status.temp1_ok ? `${fixed(status.temperature1)} °C` : "Kein plausibles Signal"),
      item("TMP36 Sensor 2", "SENSORIK", status.temp2_ok, status.temp2_ok ? "OK" : "OFFEN", status.temp2_ok ? `${fixed(status.temperature2)} °C` : "Nicht verfügbar"),
      item("TLA2024 ADC", "MESSUNG", status.tla2024_ok, status.tla2024_ok ? "OK" : "OFFEN", "Strom ×2 · Licht"),
      item("Lichtsensor", "SENSORIK", status.light_ok, status.light_ok ? status.night ? "NACHT" : "TAG" : "OFFEN", status.light_ok ? `Level ${fixed(status.light_level, 2)}` : "Kein Messwert"),
      item("Lüfter PWM", "AKTORIK", inactive || status.fan_percent > 0, inactive ? "INAKTIV" : "AKTIV", `${fixed(status.fan_percent, 0)} %`),
      item("Fan-Tacho", "MESSUNG", inactive || status.fan_rpm > 100, inactive ? "INAKTIV" : status.fan_rpm > 100 ? "OK" : "FEHLER", `${fixed(status.fan_rpm, 0)} rpm`),
      item("Peltier 1", "AKTORIK", status.current1_ok || inactive, inactive ? "SICHER AUS" : "HEIZT", `${fixed(status.power, 0)} % · ${fixed(status.current1, 2)} A`),
      item("Peltier 2", "AKTORIK", status.current2_ok || inactive, inactive ? "SICHER AUS" : "HEIZT", `${fixed(status.power, 0)} % · ${fixed(status.current2, 2)} A`),
      item("Strommessung P1", "MESSUNG", status.current1_ok, status.current1_ok ? "OK" : "OFFEN", `${fixed(status.current1, 3)} A`),
      item("Strommessung P2", "MESSUNG", status.current2_ok, status.current2_ok ? "OK" : "OFFEN", `${fixed(status.current2, 3)} A`),
      item("Bechererkennung", "SAFETY", status.cup, status.cup ? "ERKANNT" : "GESPERRT", "S_DETECT"),
      item("PG_5V0", "VERSORGUNG", status.power_good, status.power_good ? "FREI" : "NICHT FREI", status.power_good ? "Leistungspfad verfügbar" : "USB-only / keine Last"),
      item("RGB-Ring", "STATUS", status.leds_initialized, status.leds_initialized ? "OK" : "OFFEN", `Gesamtzustand ${status.state}`),
      item("Status-LEDs", "STATUS", status.leds_initialized, status.leds_initialized ? "OK" : "OFFEN", "READY · HEATING · HOLDING · HTTP · ERROR"),
      item("Taster", "INTERFACE", true, "AKTIV", "MODE · DOWN · OK · UP · DETECT"),
      item("CYW43 WLAN", "NETZWERK", status.wifi, status.wifi ? "VERBUNDEN" : "GETRENNT", `${status.wifi_ssid} · ${status.wifi_ip}`),
      item("HTTP-Webserver", "NETZWERK", status.webserver_ready, status.webserver_ready ? "BEREIT" : "OFFLINE", status.webserver_ready ? `http://${status.wifi_ip}` : "Listener nicht bestätigt")
    ];
  }

  function renderHardware(status) {
    const values = componentData(status);
    const group = name => values.filter(item => item.group === name);
    const groupText = names => {
      const items = names.flatMap(group);
      return `${items.filter(item => item.ok).length}/${items.length} NOMINAL`;
    };
    setText("clusterSensors", groupText(["SENSORIK"]));
    setText("clusterMeasurement", groupText(["MESSUNG"]));
    setText("clusterInterface", groupText(["INTERFACE"]));
    setText("clusterActuation", status.power > 0 ? `${fixed(status.power, 0)} % AKTIV` : "SICHER AUS");
    setText("clusterLight", status.leds_initialized ? "SYNCHRON" : "OFFEN");
    setText("clusterNetwork", status.webserver_ready ? "LIVE / HTTP" : "OFFLINE");
    setText("clusterPower", status.power_good ? "LAST FREI" : "USB / GESPERRT");
    setText("picoCoreStatus", runtime.online ? "CONTROL CORE ONLINE" : "SIGNAL OFFLINE");
    all(".hardware-cluster").forEach(node => {
      const key = node.dataset.cluster;
      const checks = {
        sensors: status.sensor_ok,
        measurement: status.current_ok,
        interface: status.display_initialized,
        actuation: status.state !== "FEHLER",
        light: status.leds_initialized,
        network: status.webserver_ready,
        power: status.power_good
      };
      node.classList.toggle("fault", !checks[key]);
    });
    const inspection = values.map(value => {
      const node = document.createElement("article");
      node.className = `component ${value.ok ? "ok" : "bad"}`;
      node.innerHTML = `<span>${value.group}</span><strong>${value.name}</strong><b>${value.state}</b><small>${value.detail}</small>`;
      return node;
    });
    byId("componentInspection").replaceChildren(...inspection);
  }

  function renderTechnical(status) {
    const values = [
      ["Zustand", status.state], ["Fehlercode", status.fault], ["Isttemperatur y(t)", status.sensor_ok ? `${fixed(status.temperature)} °C` : "nicht verfügbar"],
      ["Sensor 1", status.temp1_ok ? `${fixed(status.temperature1)} °C` : "nicht verfügbar"], ["Sensor 2", status.temp2_ok ? `${fixed(status.temperature2)} °C` : "nicht verfügbar"],
      ["Sollwert w(t)", `${fixed(status.setpoint)} °C`], ["Abweichung e(t)", Number.isFinite(status.error) ? `${fixed(status.error)} K` : "nicht verfügbar"],
      ["Stellgröße u(t)", `${fixed(status.power)} %`], ["Strom P1", status.current1_ok ? `${fixed(status.current1, 3)} A` : "nicht verfügbar"],
      ["Strom P2", status.current2_ok ? `${fixed(status.current2, 3)} A` : "nicht verfügbar"], ["Lüfter", `${fixed(status.fan_rpm, 0)} rpm · ${fixed(status.fan_percent, 0)} %`],
      ["Licht", status.light_ok ? `${fixed(status.light_level, 3)} · ${status.night ? "Nacht" : "Tag"}` : "nicht verfügbar"], ["Becher", status.cup ? "erkannt" : "nicht erkannt"],
      ["PG_5V0", status.power_good ? "freigegeben" : "nicht freigegeben"], ["WLAN", status.wifi ? "verbunden" : "getrennt"], ["SSID", status.wifi_ssid],
      ["IPv4", status.wifi_ip], ["HTTP", status.webserver_ready ? "bereit" : "nicht verfügbar"], ["Uptime", formatDuration(status.uptime_ms)],
      ["Max. Temperatur", `${fixed(status.max_safe_temperature)} °C`]
    ];
    byId("technicalValues").replaceChildren(...values.map(([label, value]) => {
      const row = document.createElement("div");
      row.innerHTML = `<span>${label}</span><strong>${value}</strong>`;
      return row;
    }));
  }

  function updateMetrics(status) {
    if (runtime.initialTemperature === null && status.sensor_ok) runtime.initialTemperature = status.temperature;
    if (Number.isFinite(status.error)) runtime.maxError = Math.max(runtime.maxError, Math.abs(status.error));
    if (status.state === "AUFHEIZEN" && !runtime.heatingStarted) runtime.heatingStarted = Date.now();
    if (status.state === "HALTEN" && !runtime.holdingStarted) runtime.holdingStarted = Date.now();
    if (status.state === "AUS" || status.state === "BEREIT" || status.state === "FEHLER") {
      runtime.heatingStarted = 0;
      runtime.holdingStarted = 0;
    }
    const recent = history.length > 10 ? history[Math.max(0, history.length - 11)] : null;
    const rate = recent && status.sensor_ok ? (status.temperature - recent.temperature) / Math.max(.001, (Date.now() - recent.time) / 60000) : NaN;
    setText("maxError", runtime.maxError ? `${fixed(runtime.maxError)} K` : "—");
    setText("temperatureRise", runtime.initialTemperature !== null && status.sensor_ok ? `${fixed(status.temperature - runtime.initialTemperature)} K` : "—");
    setText("temperatureRate", Number.isFinite(rate) ? `${rate >= 0 ? "+" : ""}${fixed(rate, 2)} K/min` : "—");
    setText("heatingTime", runtime.heatingStarted ? formatDuration(Date.now() - runtime.heatingStarted) : "—");
    setText("holdingTime", runtime.holdingStarted ? formatDuration(Date.now() - runtime.holdingStarted) : "—");
  }

  function addHistory(status) {
    history.push({ time: Date.now(), temperature: status.temperature, temperature2: status.temperature2, target: status.setpoint, power: status.power, valid: status.sensor_ok, valid2: status.temp2_ok });
    const earliest = Date.now() - MAX_HISTORY_SECONDS * 1000;
    while (history.length && history[0].time < earliest) history.shift();
    scheduleChart();
  }

  function scheduleChart() {
    if (runtime.drawQueued) return;
    runtime.drawQueued = true;
    requestAnimationFrame(() => { runtime.drawQueued = false; drawChart(); });
  }

  function drawChart() {
    const canvas = byId("historyCanvas");
    if (!canvas) return;
    const rect = canvas.getBoundingClientRect();
    const ratio = Math.min(devicePixelRatio || 1, 2);
    const width = Math.max(1, Math.round(rect.width * ratio));
    const height = Math.max(1, Math.round(rect.height * ratio));
    if (canvas.width !== width || canvas.height !== height) { canvas.width = width; canvas.height = height; }
    const ctx = canvas.getContext("2d");
    ctx.setTransform(ratio, 0, 0, ratio, 0, 0);
    const w = rect.width, h = rect.height, left = 52, right = 46, top = 28, bottom = 38;
    ctx.clearRect(0, 0, w, h);
    const visibleFrom = Date.now() - runtime.range * 1000;
    const data = history.filter(point => point.time >= visibleFrom);
    const validTemperatures = data.flatMap(point => [point.valid ? point.temperature : NaN, point.valid2 ? point.temperature2 : NaN, point.target]).filter(Number.isFinite);
    const minTemperature = validTemperatures.length ? Math.floor(Math.min(...validTemperatures) - 3) : 20;
    const maxTemperature = validTemperatures.length ? Math.ceil(Math.max(...validTemperatures) + 3) : 60;
    const tempSpan = Math.max(8, maxTemperature - minTemperature);
    const x = time => left + (time - visibleFrom) / (runtime.range * 1000) * (w - left - right);
    const y = value => top + (maxTemperature - value) / tempSpan * (h - top - bottom);
    ctx.font = "10px Segoe UI, sans-serif";
    ctx.textBaseline = "middle";
    for (let i = 0; i <= 4; i++) {
      const yy = top + i / 4 * (h - top - bottom);
      ctx.strokeStyle = "rgba(82,137,158,.16)"; ctx.lineWidth = 1; ctx.beginPath(); ctx.moveTo(left, yy); ctx.lineTo(w - right, yy); ctx.stroke();
      ctx.fillStyle = "rgba(132,166,179,.72)"; ctx.textAlign = "right"; ctx.fillText(`${fixed(maxTemperature - tempSpan * i / 4, 0)}°`, left - 10, yy);
      ctx.textAlign = "left"; ctx.fillText(`${100 - i * 25}%`, w - right + 10, yy);
    }
    for (let i = 0; i <= 5; i++) {
      const xx = left + i / 5 * (w - left - right);
      ctx.strokeStyle = "rgba(82,137,158,.09)"; ctx.beginPath(); ctx.moveTo(xx, top); ctx.lineTo(xx, h - bottom); ctx.stroke();
      const ago = runtime.range * (5 - i) / 5;
      ctx.fillStyle = "rgba(132,166,179,.6)"; ctx.textAlign = "center"; ctx.fillText(i === 5 ? "JETZT" : `−${ago >= 60 ? fixed(ago / 60, 0) + "m" : fixed(ago, 0) + "s"}`, xx, h - 15);
    }
    const line = (key, color, widthLine, predicate = () => true) => {
      ctx.beginPath(); let started = false;
      data.forEach(point => {
        const value = point[key];
        if (!Number.isFinite(value) || !predicate(point)) { started = false; return; }
        const px = x(point.time), py = y(value);
        if (!started) ctx.moveTo(px, py); else ctx.lineTo(px, py);
        started = true;
      });
      ctx.strokeStyle = color; ctx.lineWidth = widthLine; ctx.lineJoin = "round"; ctx.lineCap = "round"; ctx.stroke();
    };
    line("target", "rgba(103,223,241,.56)", 1.5);
    line("temperature2", "rgba(151,164,199,.58)", 1.25, point => point.valid2);
    if (data.length > 1) {
      ctx.save();
      const gradient = ctx.createLinearGradient(0, top, 0, h - bottom);
      gradient.addColorStop(0, "rgba(255,152,84,.19)"); gradient.addColorStop(1, "rgba(255,152,84,0)");
      ctx.beginPath(); let started = false;
      data.forEach(point => { if (!point.valid) return; const px = x(point.time), py = y(point.temperature); if (!started) ctx.moveTo(px, h - bottom), ctx.lineTo(px, py); else ctx.lineTo(px, py); started = true; });
      if (started) { const last = data[data.length - 1]; ctx.lineTo(x(last.time), h - bottom); ctx.closePath(); ctx.fillStyle = gradient; ctx.fill(); }
      ctx.restore();
    }
    line("temperature", "#ffad68", 2.5, point => point.valid);
    ctx.beginPath(); let powerStarted = false;
    data.forEach(point => { const px = x(point.time), py = top + (100 - point.power) / 100 * (h - top - bottom); if (!powerStarted) ctx.moveTo(px, py); else ctx.lineTo(px, py); powerStarted = true; });
    ctx.strokeStyle = "rgba(104,223,166,.62)"; ctx.lineWidth = 1.5; ctx.stroke();
  }

  function render(statusInput, options = {}) {
    const status = normalize(statusInput);
    runtime.current = status;
    document.body.dataset.state = status.state;
    document.body.classList.remove("is-loading");
    const active = status.state === "AUFHEIZEN" || status.state === "HALTEN";
    const temperatureAvailable = status.sensor_ok && Number.isFinite(status.temperature);
    const controlsAvailable = runtime.online;
    setText("networkIdentity", status.wifi ? status.wifi_ssid : "WLAN getrennt");
    setText("ipAddress", status.wifi_ip !== "—" ? status.wifi_ip : "—");
    setText("lastUpdate", runtime.lastUpdate ? new Date(runtime.lastUpdate).toLocaleTimeString("de-DE") : "—");
    setText("mainTemperature", temperatureAvailable ? fixed(status.temperature) : "—");
    setText("temperatureQuality", temperatureAvailable ? `${status.night ? "NACHTMODUS" : "TAGMODUS"} · SENSOR PLAUSIBEL` : "MESSWERT NICHT VERFÜGBAR");
    available("mainTemperature", temperatureAvailable);
    setText("sensorOneValue", status.temp1_ok ? `${fixed(status.temperature1)} °C` : "NICHT VERFÜGBAR");
    setText("plateState", active ? status.state === "HALTEN" ? "STABILISIERT" : "WÄRMEEINTRAG" : "SICHER AUS");
    setText("peltierCurrent", status.current_ok ? `${fixed(status.current1, 2)} / ${fixed(status.current2, 2)} A` : "MESSUNG OFFEN");
    setText("fanValue", `${fixed(status.fan_rpm, 0)} rpm`);
    setText("systemState", status.state);
    setText("stateDescription", stateDescriptions[status.state]);
    all("[data-for-state]").forEach(node => node.classList.toggle("active", node.dataset.forState === status.state));
    const angle = 36 + clamp((status.setpoint - 20) / 40, 0, 1) * 288;
    byId("setpointDial").style.setProperty("--set-angle", `${angle}deg`);
    byId("setpointDial").style.setProperty("--output", status.power);
    if (document.activeElement !== byId("targetInput")) byId("targetInput").value = status.setpoint.toFixed(1);
    setText("errorValue", Number.isFinite(status.error) ? `${status.error >= 0 ? "+" : ""}${fixed(status.error)} K` : "—");
    available("errorValue", Number.isFinite(status.error));
    setText("powerValue", `${fixed(status.power, 0)} %`);
    byId("powerBar").style.width = `${status.power}%`;
    setText("loopSetpoint", `${fixed(status.setpoint)} °C`);
    setText("loopError", Number.isFinite(status.error) ? `${status.error >= 0 ? "+" : ""}${fixed(status.error)} K` : "—");
    setText("loopController", active ? "REGELT" : "INAKTIV");
    setText("loopOutput", `${fixed(status.power, 0)} %`);
    setText("loopPeltier", `${fixed(status.power, 0)} %`);
    setText("loopPlant", active ? status.state === "HALTEN" ? "STABIL" : "ERWÄRMT" : "STILLSTAND");
    setText("loopSensor", temperatureAvailable ? `${fixed(status.temperature)} °C` : "—");
    byId("loopVisual").classList.toggle("flowing", active && status.power > 0 && runtime.online);
    setText("piOutput", `${fixed(status.power)} %`);
    setText("piKp", fixed(status.kp, 3)); setText("piKi", fixed(status.ki, 3));
    setText("piP", `${fixed(status.p_term)} %`); setText("piI", `${fixed(status.i_term)} %`);
    setText("piAntiWindup", status.anti_windup ? "AKTIV" : status.output_limited ? "AUSGANG BEGRENZT" : "IN BEREITSCHAFT");
    setText("controlPeriod", `${fixed(status.control_period_ms, 0)} ms`);
    byId("piRadial").style.setProperty("--pi-output", status.power);
    const startCaption = !controlsAvailable ? "Keine Live-Verbindung" : status.start_allowed ? "Alle Freigaben bestätigt" : status.start_block_reason;
    setText("startCaption", startCaption);
    byId("startButton").disabled = !controlsAvailable || !status.start_allowed || active;
    byId("stopButton").disabled = !controlsAvailable;
    byId("setpointButton").disabled = !controlsAvailable;
    byId("targetDown").disabled = !controlsAvailable;
    byId("targetUp").disabled = !controlsAvailable;
    byId("targetInput").disabled = !controlsAvailable;
    const fault = status.state === "FEHLER" || status.fault !== "KEIN FEHLER";
    byId("systemAlert").classList.toggle("visible", fault);
    setText("alertTitle", fault ? status.fault : "System nominal");
    setText("alertText", fault ? status.fault_description : "Alle Schutzfunktionen aktiv");
    document.documentElement.style.setProperty("--fan-speed", `${Math.max(.55, 3.5 - status.fan_percent * .028)}s`);
    byId("liquidFill").style.transform = `translateY(${clamp((50 - finite(status.temperature, 20)) * .45, 0, 12)}px)`;
    byId("assemblyStage").style.setProperty("--thermal-power", status.power);
    byId("assemblyStage").style.setProperty("--fan-duration", status.fan_percent > 0 ? `${Math.max(.45, 3.2 - status.fan_percent * .028)}s` : "0s");
    renderSafety(status);
    renderHardware(status);
    renderTechnical(status);
    if (options.record !== false) {
      deriveEvents(status);
      updateMetrics(status);
      addHistory(status);
    }
    if (window.ThermalField) window.ThermalField.update(status.state, status.power, status.temperature);
    runtime.initial = false;
  }

  async function fetchWithTimeout(url, options = {}) {
    const controller = new AbortController();
    const timeout = setTimeout(() => controller.abort(), 1800);
    try { return await fetch(url, { cache: "no-store", ...options, signal: controller.signal }); }
    finally { clearTimeout(timeout); }
  }

  async function poll() {
    if (PREVIEW || runtime.busy) return;
    runtime.busy = true;
    try {
      const response = await fetchWithTimeout("/api/status");
      if (!response.ok) throw new Error(`HTTP ${response.status}`);
      const status = await response.json();
      runtime.lastUpdate = Date.now();
      setOnline(true);
      render(status);
    } catch (error) {
      setOnline(false, runtime.initial ? "Warte auf Pico W" : "Live-API nicht erreichbar");
    } finally { runtime.busy = false; }
  }

  async function command(path) {
    if (!runtime.online) return;
    if (PREVIEW && window.PreviewDriver) {
      window.PreviewDriver.command(path);
      return;
    }
    const controls = ["startButton", "stopButton", "setpointButton", "targetDown", "targetUp"].map(byId);
    controls.forEach(control => { if (control) control.disabled = true; });
    try {
      const response = await fetchWithTimeout(`/api/${path}`, { method: "POST" });
      const result = await response.json().catch(() => ({}));
      if (!response.ok) throw new Error(result.reason || `HTTP ${response.status}`);
      addEvent(path === "start" ? "START an Pico übertragen" : path === "stop" ? "STOP an Pico übertragen" : "Sollwert an Pico übertragen", path === "stop" ? "warn" : "good");
      await poll();
    } catch (error) {
      addEvent(`Befehl abgelehnt: ${error.message}`, "bad");
      setOnline(false, "Befehl konnte nicht bestätigt werden");
    } finally { if (runtime.current) render(runtime.current, { record: false }); }
  }

  function setpointDelta(delta) {
    const input = byId("targetInput");
    input.value = clamp(finite(input.value, 45) + delta, 20, 60).toFixed(1);
    const angle = 36 + (finite(input.value) - 20) / 40 * 288;
    byId("setpointDial").style.setProperty("--set-angle", `${angle}deg`);
  }

  function setpointCommand() {
    const value = clamp(finite(byId("targetInput").value, 45), 20, 60);
    byId("targetInput").value = value.toFixed(1);
    command(`setpoint?value=${encodeURIComponent(value.toFixed(1))}`);
  }

  function openEngineering(open) {
    byId("engineeringDrawer").classList.toggle("open", open);
    byId("engineeringDrawer").setAttribute("aria-hidden", String(!open));
    byId("engineeringOpen").setAttribute("aria-expanded", String(open));
    byId("drawerScrim").classList.toggle("visible", open);
    if (open) byId("engineeringClose").focus(); else byId("engineeringOpen").focus();
  }

  function togglePresentation() {
    const active = !document.body.classList.contains("presentation");
    document.body.classList.toggle("presentation", active);
    byId("presentationToggle").setAttribute("aria-pressed", String(active));
    setText("presentationLabel", active ? "PRÄSENTATION BEENDEN" : "PRÄSENTATION");
    if (active && document.documentElement.requestFullscreen) document.documentElement.requestFullscreen().catch(() => {});
    if (!active && document.fullscreenElement) document.exitFullscreen().catch(() => {});
    scheduleChart();
  }

  function initialiseInteractions() {
    byId("startButton").addEventListener("click", () => command("start"));
    byId("stopButton").addEventListener("click", () => command("stop"));
    byId("setpointButton").addEventListener("click", setpointCommand);
    byId("targetDown").addEventListener("click", () => setpointDelta(-.5));
    byId("targetUp").addEventListener("click", () => setpointDelta(.5));
    byId("targetInput").addEventListener("change", () => setpointDelta(0));
    byId("targetInput").addEventListener("keydown", event => { if (event.key === "Enter") setpointCommand(); });
    byId("engineeringOpen").addEventListener("click", () => openEngineering(true));
    byId("hardwareInspect").addEventListener("click", () => openEngineering(true));
    byId("engineeringClose").addEventListener("click", () => openEngineering(false));
    byId("drawerScrim").addEventListener("click", () => openEngineering(false));
    byId("presentationToggle").addEventListener("click", togglePresentation);
    byId("motionToggle").addEventListener("click", () => {
      runtime.lowMotion = !runtime.lowMotion;
      document.body.classList.toggle("low-motion", runtime.lowMotion);
      byId("motionToggle").setAttribute("aria-pressed", String(runtime.lowMotion));
      if (window.ThermalField) window.ThermalField.setLowMotion(runtime.lowMotion);
    });
    all(".range-button").forEach(button => button.addEventListener("click", () => {
      runtime.range = finite(button.dataset.range, 60);
      all(".range-button").forEach(item => item.classList.toggle("active", item === button));
      scheduleChart();
    }));
    addEventListener("resize", scheduleChart, { passive: true });
    addEventListener("keydown", event => { if (event.key === "Escape") openEngineering(false); });
    document.addEventListener("fullscreenchange", () => {
      if (!document.fullscreenElement && document.body.classList.contains("presentation")) {
        document.body.classList.remove("presentation");
        byId("presentationToggle").setAttribute("aria-pressed", "false");
        setText("presentationLabel", "PRÄSENTATION");
      }
    });
  }

  function boot() {
    initialiseInteractions();
    document.body.classList.toggle("low-motion", runtime.lowMotion);
    byId("motionToggle").setAttribute("aria-pressed", String(runtime.lowMotion));
    if (window.ThermalField) {
      window.ThermalField.initialise(byId("thermalField"));
      window.ThermalField.setLowMotion(runtime.lowMotion);
    }
    const skipIntro = runtime.lowMotion || new URLSearchParams(location.search).has("review");
    if (skipIntro) byId("bootSequence").classList.add("done");
    else setTimeout(() => byId("bootSequence").classList.add("done"), 1350);
    if (PREVIEW && window.PreviewDriver) window.PreviewDriver.start();
    else {
      poll();
      setInterval(poll, SAMPLE_INTERVAL);
    }
  }

  window.ThermalUI = {
    acceptStatus(status, online = true) {
      runtime.lastUpdate = Date.now();
      setOnline(online, online ? "" : "Vorschau: Signal unterbrochen");
      render(status);
    },
    setOnline,
    addEvent,
    command,
    history,
    runtime
  };

  if (document.readyState === "loading") document.addEventListener("DOMContentLoaded", boot, { once: true });
  else boot();
}());
