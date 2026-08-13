(function () {
  "use strict";

  const stateColors = {
    AUS: [0.18, 0.31, 0.37],
    BEREIT: [0.17, 0.55, 0.88],
    AUFHEIZEN: [1.0, 0.36, 0.11],
    HALTEN: [0.20, 0.79, 0.49],
    FEHLER: [1.0, 0.16, 0.24]
  };
  let canvas;
  let gl;
  let context2d;
  let program;
  let uniforms;
  let frame = 0;
  let lastFrame = 0;
  let state = "AUS";
  let power = 0;
  let temperature = 20;
  let lowMotion = matchMedia("(prefers-reduced-motion: reduce)").matches;

  const vertexSource = `
    attribute vec2 p;
    varying vec2 uv;
    void main(){uv=p*.5+.5;gl_Position=vec4(p,0.,1.);}
  `;
  const fragmentSource = `
    precision mediump float;
    varying vec2 uv;
    uniform vec2 resolution;
    uniform float time;
    uniform float outputPower;
    uniform float processTemperature;
    uniform vec3 stateColor;
    float hash(vec2 p){return fract(sin(dot(p,vec2(127.1,311.7)))*43758.5453123);}
    float noise(vec2 p){
      vec2 i=floor(p),f=fract(p);f=f*f*(3.-2.*f);
      return mix(mix(hash(i),hash(i+vec2(1.,0.)),f.x),mix(hash(i+vec2(0.,1.)),hash(i+vec2(1.)),f.x),f.y);
    }
    float fbm(vec2 p){float v=0.;v+=noise(p)*.55;p=p*2.03+9.2;v+=noise(p)*.28;p=p*2.01+2.7;v+=noise(p)*.14;return v;}
    void main(){
      vec2 q=uv;vec2 aspect=vec2(resolution.x/max(resolution.y,1.),1.);
      vec2 p=(q-.5)*aspect;
      float motion=time*(.035+.09*outputPower);
      float plume=fbm(vec2(p.x*2.1,p.y*1.15-motion));
      float vessel=exp(-length((p-vec2(-.12,.02))*vec2(.78,1.18))*3.1);
      float heat=exp(-length((p-vec2(-.12,-.34))*vec2(.7,1.55))*3.7);
      float trace=smoothstep(.82,.16,abs(p.y-.08*sin(p.x*5.5+motion*4.))+.5*abs(p.x));
      float energy=(.12+outputPower*.78)*(heat*.68+plume*vessel*.32);
      vec3 cold=vec3(.008,.027,.043);
      vec3 thermal=mix(vec3(.035,.14,.19),stateColor,clamp(energy+trace*.035,0.,1.));
      vec3 color=mix(cold,thermal,clamp(vessel*.20+energy+plume*.035,0.,.72));
      color+=stateColor*trace*.022;
      color*=.91+.09*smoothstep(20.,60.,processTemperature);
      float vignette=smoothstep(1.15,.25,length(p));color*=mix(.55,1.,vignette);
      gl_FragColor=vec4(color,1.);
    }
  `;

  function compile(type, source) {
    const shader = gl.createShader(type);
    gl.shaderSource(shader, source);
    gl.compileShader(shader);
    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) throw new Error(gl.getShaderInfoLog(shader));
    return shader;
  }

  function initialiseWebGL() {
    gl = canvas.getContext("webgl", { alpha: false, antialias: false, depth: false, powerPreference: "low-power" });
    if (!gl) return false;
    program = gl.createProgram();
    gl.attachShader(program, compile(gl.VERTEX_SHADER, vertexSource));
    gl.attachShader(program, compile(gl.FRAGMENT_SHADER, fragmentSource));
    gl.linkProgram(program);
    if (!gl.getProgramParameter(program, gl.LINK_STATUS)) throw new Error(gl.getProgramInfoLog(program));
    gl.useProgram(program);
    const buffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, buffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([-1, -1, 3, -1, -1, 3]), gl.STATIC_DRAW);
    const position = gl.getAttribLocation(program, "p");
    gl.enableVertexAttribArray(position);
    gl.vertexAttribPointer(position, 2, gl.FLOAT, false, 0, 0);
    uniforms = {
      resolution: gl.getUniformLocation(program, "resolution"),
      time: gl.getUniformLocation(program, "time"),
      power: gl.getUniformLocation(program, "outputPower"),
      temperature: gl.getUniformLocation(program, "processTemperature"),
      color: gl.getUniformLocation(program, "stateColor")
    };
    return true;
  }

  function resize() {
    const ratio = Math.min(devicePixelRatio || 1, 1.35);
    const width = Math.max(1, Math.floor(innerWidth * ratio));
    const height = Math.max(1, Math.floor(innerHeight * ratio));
    if (canvas.width === width && canvas.height === height) return;
    canvas.width = width;
    canvas.height = height;
    if (gl) gl.viewport(0, 0, width, height);
  }

  function drawFallback() {
    resize();
    const ctx = context2d;
    if (!ctx) {
      canvas.style.background = "radial-gradient(circle at 43% 62%, rgba(255,152,84,.09), transparent 58%), #050b11";
      return;
    }
    const width = canvas.width;
    const height = canvas.height;
    const color = stateColors[state] || stateColors.AUS;
    ctx.fillStyle = "#050b11";
    ctx.fillRect(0, 0, width, height);
    const gradient = ctx.createRadialGradient(width * .43, height * .62, 0, width * .43, height * .62, Math.max(width, height) * .64);
    const rgb = color.map(value => Math.round(value * 255)).join(",");
    gradient.addColorStop(0, `rgba(${rgb},${.08 + power * .0018})`);
    gradient.addColorStop(.48, `rgba(${rgb},.035)`);
    gradient.addColorStop(1, "rgba(5,11,17,0)");
    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width, height);
  }

  function render(now) {
    frame = lowMotion ? 0 : requestAnimationFrame(render);
    if (document.hidden || (!lowMotion && now - lastFrame < 33)) return;
    lastFrame = now;
    resize();
    if (!gl) return drawFallback();
    const color = stateColors[state] || stateColors.AUS;
    gl.uniform2f(uniforms.resolution, canvas.width, canvas.height);
    gl.uniform1f(uniforms.time, lowMotion ? 0 : now * .001);
    gl.uniform1f(uniforms.power, power / 100);
    gl.uniform1f(uniforms.temperature, temperature);
    gl.uniform3fv(uniforms.color, color);
    gl.drawArrays(gl.TRIANGLES, 0, 3);
  }

  function initialise(target) {
    canvas = target;
    if (!canvas) return;
    try {
      if (!initialiseWebGL()) context2d = canvas.getContext("2d");
    } catch (error) {
      gl = null;
      context2d = canvas.getContext("2d");
      console.info("[UI] WebGL nicht verfügbar, Canvas-Fallback aktiv", error.message);
    }
    addEventListener("resize", resize, { passive: true });
    document.addEventListener("visibilitychange", () => { if (!document.hidden) lastFrame = 0; });
    cancelAnimationFrame(frame);
    frame = requestAnimationFrame(render);
  }

  window.ThermalField = {
    initialise,
    update(nextState, nextPower, nextTemperature) {
      state = nextState || "AUS";
      power = Math.max(0, Math.min(100, Number(nextPower) || 0));
      temperature = Number.isFinite(Number(nextTemperature)) ? Number(nextTemperature) : 20;
      if (lowMotion) { lastFrame = 0; requestAnimationFrame(render); }
    },
    setLowMotion(enabled) {
      cancelAnimationFrame(frame);
      lowMotion = Boolean(enabled);
      lastFrame = 0;
      frame = requestAnimationFrame(render);
    }
  };
}());
