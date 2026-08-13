(function(){
"use strict";
var canvas,gl,program,frame=0,last=0,observer=null;
var model={state:"AUS",power:0,temperature:22,fan:0,mode:"overview",focus:"system",lowMotion:false,pointerX:0,pointerY:0,visible:true};
var locations={};
var vertex="attribute vec2 p;void main(){gl_Position=vec4(p,0.,1.);}";
var fragment=`precision highp float;
uniform vec2 r;
uniform float t,power,temp,state,explode,thermal,fan,focus;
uniform vec2 pointer;
#define MAX_STEPS 88
#define FAR 18.0
mat2 rot(float a){float c=cos(a),s=sin(a);return mat2(c,-s,s,c);}
float box(vec3 p,vec3 b){vec3 q=abs(p)-b;return length(max(q,0.))+min(max(q.x,max(q.y,q.z)),0.);}
float cyl(vec3 p,float h,float ra){vec2 d=abs(vec2(length(p.xz),p.y))-vec2(ra,h);return min(max(d.x,d.y),0.)+length(max(d,0.));}
float torus(vec3 p,vec2 q){return length(vec2(length(p.xz)-q.x,p.y))-q.y;}
vec2 join(vec2 a,vec2 b){return a.x<b.x?a:b;}
vec2 map(vec3 p){
 float ex=explode;
 vec2 z=vec2(99.,0.);
 vec3 q=p;
 q.y-=1.28+ex*.68;
 float outer=cyl(q,1.37,1.18);
 float inner=cyl(q-vec3(0.,.14,0.),1.3,1.02);
 float cup=max(outer,-inner);
 cup=max(cup,-(p.y-(2.57+ex*.68)));
 z=join(z,vec2(cup,1.));
 z=join(z,vec2(torus(q-vec3(0.,1.34,0.),vec2(1.1,.07)),2.));
 vec3 li=p-vec3(0.,.86+ex*.62,0.);
 z=join(z,vec2(cyl(li,.82,.99),3.));
 vec3 plate=p-vec3(0.,-.30-ex*.14,0.);
 z=join(z,vec2(cyl(plate,.13,1.44),4.));
 vec3 pel=p-vec3(0.,-.67-ex*.5,0.);
 z=join(z,vec2(box(pel,vec3(.88,.20,.82)),5.));
 vec3 sink=p-vec3(0.,-1.08-ex*.82,0.);
 z=join(z,vec2(box(sink,vec3(.98,.16,.9)),6.));
 for(int i=0;i<7;i++){float fi=float(i)-3.;z=join(z,vec2(box(sink-vec3(fi*.25,-.34,0.),vec3(.055,.35,.86)),6.));}
 vec3 fp=p-vec3(0.,-1.82-ex*1.15,.0);
 fp.xz=rot(t*fan*.85)*fp.xz;
 z=join(z,vec2(cyl(fp,.08,.25),7.));
 for(int i=0;i<5;i++){float a=float(i)*1.2566;vec3 blade=fp;blade.xz=rot(a)*blade.xz;blade.x-=.52;z=join(z,vec2(box(blade,vec3(.38,.07,.12)),7.));}
 vec3 probe=p-vec3(1.30+ex*.42,1.18+ex*.38,0.);
 z=join(z,vec2(cyl(probe,.9,.045),8.));
 z=join(z,vec2(length(probe-vec3(0.,-.9,0.))-.10,9.));
 vec3 pcb=p-vec3(-1.62-ex*.65,-.87-ex*.24,.10);
 z=join(z,vec2(box(pcb,vec3(.5,.035,.7)),10.));
 z=join(z,vec2(box(pcb-vec3(-.18,.10,.0),vec3(.18,.08,.26)),11.));
 z=join(z,vec2(box(pcb-vec3(.12,.07,.30),vec3(.27,.025,.18)),12.));
 z=join(z,vec2(torus(pcb-vec3(-.08,.12,-.32),vec2(.16,.025)),13.));
 return z;
}
vec3 normal(vec3 p){vec2 e=vec2(.002,0.);return normalize(vec3(map(p+e.xyy).x-map(p-e.xyy).x,map(p+e.yxy).x-map(p-e.yxy).x,map(p+e.yyx).x-map(p-e.yyx).x));}
vec3 material(float m,vec3 p,vec3 n){
 vec3 c=vec3(.08,.13,.15);float rough=.7;
 if(m<1.5){c=vec3(.19,.34,.39)+pow(max(n.y,0.),4.)*.16;rough=.28;}
 else if(m<2.5){c=vec3(.48,.70,.74);rough=.22;}
 else if(m<3.5){c=mix(vec3(.20,.10,.045),vec3(1.,.39,.09),thermal*.65+clamp((temp-20.)/45.,0.,1.)*.35);rough=.42;}
 else if(m<4.5){c=mix(vec3(.14,.20,.22),vec3(1.,.19,.035),thermal);rough=.28;}
 else if(m<5.5){c=mix(vec3(.08,.18,.22),vec3(.78,.16,.035),thermal*.75);rough=.5;}
 else if(m<6.5){c=vec3(.12,.19,.21);rough=.46;}
 else if(m<7.5){c=vec3(.055,.14,.17);rough=.58;}
 else if(m<8.5){c=vec3(.23,.52,.59);rough=.28;}
 else if(m<9.5){c=state>3.5?vec3(1.,.08,.12):vec3(.12,.85,1.);rough=.18;}
 else if(m<10.5){c=vec3(.035,.27,.20);rough=.64;}
 else if(m<12.5){c=vec3(.035,.42,.52);rough=.21;}
 else{c=state>3.5?vec3(1.,.03,.06):vec3(.12,.8,.9);rough=.16;}
 if(explode>.2&&focus>1.5){float grid=step(.94,fract((p.x+p.z)*9.));c+=grid*vec3(.04,.24,.25);}
 return c*(.76+.24*(1.-rough));
}
float trace(vec3 ro,vec3 rd,out float m){float d=0.;m=0.;for(int i=0;i<MAX_STEPS;i++){vec2 h=map(ro+rd*d);m=h.y;if(h.x<.0015||d>FAR)break;d+=h.x*.76;}return d;}
void main(){
 vec2 uv=(gl_FragCoord.xy*2.-r)/r.y;
 float yaw=-.56+pointer.x*.055+explode*.12;
 float pitch=-.14+pointer.y*.035-explode*.04;
 vec3 ro=vec3(4.7+explode*1.1,3.2+explode*.55,6.8+explode*1.5);
 ro.xz=rot(yaw)*ro.xz;ro.yz=rot(pitch)*ro.yz;
 vec3 ta=vec3(0.,.25,0.);vec3 ww=normalize(ta-ro),uu=normalize(cross(ww,vec3(0.,1.,0.))),vv=cross(uu,ww);
 vec3 rd=normalize(ww*2.25+uv.x*uu+uv.y*vv);
 float m;float d=trace(ro,rd,m);vec3 bg=mix(vec3(.006,.018,.025),vec3(.025,.075,.09),max(0.,uv.y*.5+.3));
 float halo=exp(-length(uv-vec2(0.,-.23))*3.4)*thermal;bg+=vec3(.30,.055,.01)*halo;
 if(d>FAR){gl_FragColor=vec4(bg,1.);return;}
 vec3 pos=ro+rd*d,n=normal(pos);vec3 key=normalize(vec3(-.6,.8,.45)),rim=normalize(vec3(.8,.28,-.6));
 float diff=max(dot(n,key),0.),edge=pow(1.-max(dot(-rd,n),0.),3.),spec=pow(max(dot(reflect(-key,n),-rd),0.),24.);
 vec3 c=material(m,pos,n);c*=.34+diff*.96;c+=spec*.38+edge*vec3(.15,.36,.42);
 float heat=max(0.,1.-abs(pos.y+.25)*.65)*thermal;c+=vec3(.55,.085,.01)*heat*.34;
 float red=state>3.5?.55:0.;c+=red*vec3(.65,.015,.02)*edge;
 float fog=1.-exp(-d*d*.009);c=mix(c,bg,fog);
 c=c/(c+vec3(1.));c=pow(c,vec3(.82));
 float vignette=smoothstep(1.48,.28,length(uv*vec2(.72,1.)));gl_FragColor=vec4(c*vignette,1.);
}`;
function shader(type,source){var s=gl.createShader(type);gl.shaderSource(s,source);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw new Error(gl.getShaderInfoLog(s));return s;}
function initialise(target){
 canvas=target;if(!canvas)return false;
 try{
  gl=canvas.getContext("webgl",{alpha:true,antialias:false,powerPreference:"high-performance"});if(!gl)return false;
  program=gl.createProgram();gl.attachShader(program,shader(gl.VERTEX_SHADER,vertex));gl.attachShader(program,shader(gl.FRAGMENT_SHADER,fragment));gl.linkProgram(program);if(!gl.getProgramParameter(program,gl.LINK_STATUS))throw new Error(gl.getProgramInfoLog(program));
  var buffer=gl.createBuffer();gl.bindBuffer(gl.ARRAY_BUFFER,buffer);gl.bufferData(gl.ARRAY_BUFFER,new Float32Array([-1,-1,3,-1,-1,3]),gl.STATIC_DRAW);gl.useProgram(program);var point=gl.getAttribLocation(program,"p");gl.enableVertexAttribArray(point);gl.vertexAttribPointer(point,2,gl.FLOAT,false,0,0);
  ["r","t","power","temp","state","explode","thermal","fan","focus","pointer"].forEach(function(k){locations[k]=gl.getUniformLocation(program,k);});
  document.body.classList.add("webgl-ready");
  addEventListener("pointermove",function(event){if(model.lowMotion)return;model.pointerX=(event.clientX/innerWidth-.5)*2;model.pointerY=(event.clientY/innerHeight-.5)*-2;},{passive:true});
  addEventListener("resize",resize,{passive:true});
  if("IntersectionObserver" in window){observer=new IntersectionObserver(function(entries){model.visible=entries[0].isIntersecting;});observer.observe(canvas);}
  resize();frame=requestAnimationFrame(render);return true;
 }catch(error){console.warn("V3 WebGL fallback:",error);gl=null;return false;}
}
function resize(){if(!gl||!canvas)return;var rect=canvas.getBoundingClientRect();var embedded=innerWidth<1000;var ratio=Math.min(devicePixelRatio||1,embedded?1:.82);var width=Math.max(1,Math.floor(rect.width*ratio)),height=Math.max(1,Math.floor(rect.height*ratio));if(canvas.width!==width||canvas.height!==height){canvas.width=width;canvas.height=height;gl.viewport(0,0,width,height);}}
function render(now){frame=requestAnimationFrame(render);if(!gl||!model.visible)return;var cap=model.lowMotion?250:33;if(now-last<cap)return;last=now;resize();var states={AUS:0,BEREIT:1,AUFHEIZEN:2,HALTEN:3,FEHLER:4};var heat=model.state==="AUFHEIZEN"||model.state==="HALTEN"?Math.max(.06,model.power/100):0;gl.useProgram(program);gl.uniform2f(locations.r,canvas.width,canvas.height);gl.uniform1f(locations.t,model.lowMotion?0:now/1000);gl.uniform1f(locations.power,model.power/100);gl.uniform1f(locations.temp,model.temperature);gl.uniform1f(locations.state,states[model.state]||0);gl.uniform1f(locations.explode,model.mode==="engineering"?1:0);gl.uniform1f(locations.thermal,heat);gl.uniform1f(locations.fan,model.lowMotion?0:Math.min(2,model.fan/2400));gl.uniform1f(locations.focus,{system:1,sensors:2,power:3}[model.focus]||1);gl.uniform2f(locations.pointer,model.pointerX,model.pointerY);gl.drawArrays(gl.TRIANGLES,0,3);}
function update(status){model.state=status.state||"AUS";model.power=Number(status.power)||0;model.temperature=Number(status.temperature)||22;model.fan=Number(status.fan_rpm)||0;}
function setMode(mode){model.mode=mode||"overview";}
function setFocus(focus){model.focus=focus||"system";}
function setLowMotion(value){model.lowMotion=!!value;}
window.ProductScene={initialise:initialise,update:update,setMode:setMode,setFocus:setFocus,setLowMotion:setLowMotion,resize:resize};
})();
