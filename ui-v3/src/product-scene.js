(function(){
"use strict";
var canvas,gl,program,frame=0,last=0,observer=null,firstFrame=false,readyCallback=null,projectionCallback=null;
var model={state:"AUS",power:0,temperature:22,fan:0,mode:"product",lens:"product",focus:"system",explode:0,lowMotion:false,frameInterval:33,pointerX:0,pointerY:0,visible:true};
var locations={};
var vertex="attribute vec2 p;void main(){gl_Position=vec4(p,0.,1.);}";
var fragment=`precision highp float;
uniform vec2 r;
uniform float t,power,temp,state,explode,thermal,fan,focus,viewMode;
uniform vec2 pointer;
#define MAX_STEPS 104
#define FAR 20.0
mat2 rot(float a){float c=cos(a),s=sin(a);return mat2(c,-s,s,c);}
float box(vec3 p,vec3 b){vec3 q=abs(p)-b;return length(max(q,0.))+min(max(q.x,max(q.y,q.z)),0.);}
float cylY(vec3 p,float h,float ra){vec2 d=abs(vec2(length(p.xz),p.y))-vec2(ra,h);return min(max(d.x,d.y),0.)+length(max(d,0.));}
float cylZ(vec3 p,float h,float ra){vec2 d=abs(vec2(length(p.xy),p.z))-vec2(ra,h);return min(max(d.x,d.y),0.)+length(max(d,0.));}
float torusY(vec3 p,vec2 q){return length(vec2(length(p.xz)-q.x,p.y))-q.y;}
float torusZ(vec3 p,vec2 q){return length(vec2(length(p.xy)-q.x,p.z))-q.y;}
float coneY(vec3 p,float h,float r1,float r2){vec2 q=vec2(length(p.xz),p.y),k1=vec2(r2,h),k2=vec2(r2-r1,2.*h);vec2 ca=vec2(q.x-min(q.x,q.y<0.?r1:r2),abs(q.y)-h);vec2 cb=q-k1+k2*clamp(dot(k1-q,k2)/dot(k2,k2),0.,1.);float s=cb.x<0.&&ca.y<0.?-1.:1.;return s*sqrt(min(dot(ca,ca),dot(cb,cb)));}
vec2 join(vec2 a,vec2 b){return a.x<b.x?a:b;}
vec2 map(vec3 p){
 float ex=explode;vec2 z=vec2(99.,0.);
 float cupLift=ex*.72;vec3 cup=p-vec3(0.,1.25+cupLift,0.);
 float outer=coneY(cup,1.22,.84,1.02),inner=coneY(cup-vec3(0.,.11,0.),1.16,.73,.91);float glass=max(outer,-inner);
 if(viewMode<3.5||viewMode>4.5)z=join(z,vec2(glass,1.));
 z=join(z,vec2(torusY(p-vec3(0.,2.47+cupLift,0.),vec2(1.015,.055)),2.));
 z=join(z,vec2(torusY(p-vec3(0.,.43+cupLift*.18,0.),vec2(.94,.034)),15.));
 z=join(z,vec2(box(p-vec3(1.02,.20+cupLift*.12,0.),vec3(.035,.24,.07)),15.));
 z=join(z,vec2(box(p-vec3(-1.02,.20+cupLift*.12,0.),vec3(.035,.24,.07)),15.));
 vec3 liquid=p-vec3(0.,1.03+cupLift,0.);if(viewMode<3.5||viewMode>4.5)z=join(z,vec2(coneY(liquid,.87,.73,.88),3.));
 float plateY=-.09-ex*.16;z=join(z,vec2(cylY(p-vec3(0.,plateY,0.),.12,1.25),4.));
 z=join(z,vec2(torusY(p-vec3(0.,plateY+.09,0.),vec2(1.17,.025)),13.));
 vec3 pel1=p-vec3(-.51,-.32-ex*.50,0.);vec3 pel2=p-vec3(.51,-.32-ex*.50,0.);
 z=join(z,vec2(box(pel1,vec3(.47,.11,.68)),5.));z=join(z,vec2(box(pel2,vec3(.47,.11,.68)),17.));
 vec3 sink=p-vec3(0.,-.56-ex*.86,0.);z=join(z,vec2(box(sink,vec3(1.08,.12,.78)),6.));
 for(int i=0;i<9;i++){float fi=float(i)-4.;z=join(z,vec2(box(sink-vec3(fi*.23,-.39,0.),vec3(.045,.31,.75)),6.));}
 vec3 fp=p-vec3(0.,-.98-ex*1.15,.82+ex*.18);z=join(z,vec2(torusZ(fp,vec2(.47,.065)),7.));z=join(z,vec2(cylZ(fp,.075,.105),7.));
 vec3 moving=fp;moving.xy=rot(t*fan*.48)*moving.xy;for(int i=0;i<7;i++){float a=float(i)*.8976;vec3 blade=moving;blade.xy=rot(a)*blade.xy;blade.x-=.27;z=join(z,vec2(box(blade,vec3(.20,.065,.045)),7.));}
 vec3 s1=p-vec3(1.00+ex*.32,.34+ex*.28,.12);z=join(z,vec2(box(s1,vec3(.085,.13,.06)),8.));z=join(z,vec2(box(s1-vec3(-.05,-.20,0.),vec3(.018,.10,.018)),16.));z=join(z,vec2(box(s1-vec3(.05,-.20,0.),vec3(.018,.10,.018)),16.));
 vec3 s2=p-vec3(-.78-ex*.30,1.10+ex*.62,.64);z=join(z,vec2(box(s2,vec3(.075,.12,.055)),9.));z=join(z,vec2(box(s2-vec3(0.,-.20,-.05),vec3(.02,.09,.02)),16.));
 vec3 pcb=p-vec3(-1.42-ex*.72,-.33-ex*.20,.08);z=join(z,vec2(box(pcb,vec3(.46,.39,.035)),10.));z=join(z,vec2(box(pcb-vec3(.39,.12,-.08),vec3(.10,.33,.10)),15.));
 z=join(z,vec2(box(pcb-vec3(-.12,.08,.07),vec3(.15,.19,.035)),11.));z=join(z,vec2(box(pcb-vec3(.18,.17,.07),vec3(.10,.08,.035)),12.));
 z=join(z,vec2(box(pcb-vec3(.19,-.13,.07),vec3(.14,.10,.035)),18.));z=join(z,vec2(box(pcb-vec3(-.25,-.27,.07),vec3(.13,.055,.05)),18.));
 vec3 console=p-vec3(1.38+ex*.58,-.24-ex*.12,.13);z=join(z,vec2(box(console,vec3(.36,.36,.055)),19.));z=join(z,vec2(box(console-vec3(0.,.12,.075),vec3(.24,.105,.025)),20.));
 for(int i=0;i<3;i++){z=join(z,vec2(cylZ(console-vec3((float(i)-1.)*.16,-.14,.08),.035,.045),21.));}
 z=join(z,vec2(cylZ(console-vec3(.26,-.27,.08),.035,.045),22.));
 z=join(z,vec2(cylY(p-vec3(.72,.045+cupLift*.03,.53),.055,.085),23.));
 z=join(z,vec2(p.y+1.78,14.));return z;
}
vec3 normal(vec3 p){vec2 e=vec2(.002,0.);return normalize(vec3(map(p+e.xyy).x-map(p-e.xyy).x,map(p+e.yxy).x-map(p-e.yxy).x,map(p+e.yyx).x-map(p-e.yyx).x));}
vec3 material(float m,vec3 p,vec3 n){
 vec3 c=vec3(.08,.13,.15);float rough=.7;
 if(m<1.5){c=vec3(.31,.48,.51)+pow(max(n.y,0.),4.)*.12;rough=.16;}
 else if(m<2.5){c=vec3(.70,.82,.83);rough=.12;}
 else if(m<3.5){c=mix(vec3(.18,.075,.025),vec3(.86,.25,.035),thermal*.42+clamp((temp-20.)/45.,0.,1.)*.25);rough=.42;}
 else if(m<4.5){c=mix(vec3(.36,.39,.39),vec3(.95,.27,.045),thermal*.62);rough=.23;}
 else if(m<5.5){c=mix(vec3(.72,.72,.68),vec3(.62,.13,.045),thermal*.50);rough=.40;}
 else if(m<6.5){c=vec3(.32,.36,.36);rough=.38;}
 else if(m<7.5){c=vec3(.035,.045,.050);rough=.62;}
 else if(m<8.5){c=vec3(.15,.42,.45);rough=.31;}
 else if(m<9.5){c=vec3(.30,.56,.50);rough=.31;}
 else if(m<10.5){c=vec3(.025,.20,.12);rough=.67;}
 else if(m<11.5){c=vec3(.09,.42,.49);rough=.28;}
 else if(m<12.5){c=vec3(.09,.20,.25);rough=.30;}
 else if(m<13.5){c=state>3.5?vec3(1.,.025,.04):state>2.5?vec3(.10,.82,.34):state>1.5?vec3(1.,.30,.04):vec3(.08,.35,.65);rough=.18;}
 else if(m<14.5){float grid=viewMode>2.5?step(.982,fract(p.x*2.))+step(.982,fract(p.z*2.)):0.;c=vec3(.012,.019,.022)+grid*vec3(.02,.06,.065);rough=.89;}
 else if(m<16.5){c=vec3(.24,.30,.31);rough=.42;}
 else if(m<17.5){c=vec3(.35,.33,.26);rough=.48;}
 else if(m<18.5){c=mix(vec3(.76,.76,.71),vec3(.62,.12,.04),thermal*.50);rough=.42;}
 else if(m<19.5){c=vec3(.06,.07,.07);rough=.55;}
 else if(m<20.5){c=vec3(.025,.20,.24);rough=.21;}
 else if(m<21.5){c=vec3(.045,.055,.058);rough=.52;}
 else if(m<22.5){c=vec3(.18,.48,.52);rough=.18;}
 else{c=state>3.5?vec3(1.,.05,.08):vec3(.10,.72,.78);rough=.17;}
 if(focus>1.5){bool sensorPart=(m>7.5&&m<9.5)||(m>15.5&&m<16.5)||m>21.5;bool powerPart=m>3.5&&m<7.5||m>16.5&&m<18.5;if((focus<2.5&&!sensorPart)||(focus>2.5&&!powerPart))c*=.48;else c+=vec3(.06,.17,.16);}
 return c*(.76+.24*(1.-rough));
}
float trace(vec3 ro,vec3 rd,out float m){float d=0.;m=0.;for(int i=0;i<MAX_STEPS;i++){vec2 h=map(ro+rd*d);m=h.y;if(h.x<.0015||d>FAR)break;d+=h.x*.72;}return d;}
void main(){
 vec2 uv=(gl_FragCoord.xy*2.-r)/r.y;float yaw=-.56+pointer.x*.055+explode*.12;float pitch=-.14+pointer.y*.035-explode*.04;
 vec3 ro=vec3(4.7+explode*1.1,3.2+explode*.55,6.8+explode*1.5);ro.xz=rot(yaw)*ro.xz;ro.yz=rot(pitch)*ro.yz;
 vec3 ta=vec3(0.,.25,0.),ww=normalize(ta-ro),uu=normalize(cross(ww,vec3(0.,1.,0.))),vv=cross(uu,ww);vec3 rd=normalize(ww*2.55+uv.x*uu+uv.y*vv);
 float m,d=trace(ro,rd,m);vec3 bg=mix(vec3(.005,.013,.017),vec3(.022,.047,.052),max(0.,uv.y*.48+.28));float halo=exp(-length(uv-vec2(0.,-.12))*3.9)*thermal;bg+=vec3(.25,.045,.008)*halo;
 if(d>FAR){gl_FragColor=vec4(bg,1.);return;}vec3 pos=ro+rd*d,n=normal(pos),key=normalize(vec3(-.58,.82,.42));float diff=max(dot(n,key),0.),edge=pow(1.-max(dot(-rd,n),0.),3.),spec=pow(max(dot(reflect(-key,n),-rd),0.),30.);
 vec3 c=material(m,pos,n);float ao=clamp(map(pos+n*.08).x/.08,0.,1.)*.55+clamp(map(pos+n*.20).x/.20,0.,1.)*.45;c*=(.48+diff*1.05)*(.72+.28*ao);c+=spec*.40+edge*vec3(.16,.31,.32);
 if(m<1.5)c=mix(bg,c,.38+edge*.48);float heat=max(0.,1.-abs(pos.y+.18)*.75)*thermal;c+=vec3(.48,.06,.004)*heat*.28;float red=state>3.5?.48:0.;c+=red*vec3(.62,.01,.018)*edge;
 float fog=1.-exp(-d*d*.008);c=mix(c,bg,fog);c*=1.22;c=c/(c+vec3(1.));c=pow(c,vec3(.82));float vignette=smoothstep(1.48,.28,length(uv*vec2(.72,1.)));gl_FragColor=vec4(c*vignette,1.);
}`;
function shader(type,source){var s=gl.createShader(type);gl.shaderSource(s,source);gl.compileShader(s);if(!gl.getShaderParameter(s,gl.COMPILE_STATUS))throw new Error(gl.getShaderInfoLog(s));return s;}
function setup(){try{gl=canvas.getContext("webgl",{alpha:true,antialias:false,powerPreference:"high-performance"});if(!gl)return;program=gl.createProgram();gl.attachShader(program,shader(gl.VERTEX_SHADER,vertex));gl.attachShader(program,shader(gl.FRAGMENT_SHADER,fragment));gl.linkProgram(program);if(!gl.getProgramParameter(program,gl.LINK_STATUS))throw new Error(gl.getProgramInfoLog(program));var buffer=gl.createBuffer();gl.bindBuffer(gl.ARRAY_BUFFER,buffer);gl.bufferData(gl.ARRAY_BUFFER,new Float32Array([-1,-1,3,-1,-1,3]),gl.STATIC_DRAW);gl.useProgram(program);var point=gl.getAttribLocation(program,"p");gl.enableVertexAttribArray(point);gl.vertexAttribPointer(point,2,gl.FLOAT,false,0,0);["r","t","power","temp","state","explode","thermal","fan","focus","viewMode","pointer"].forEach(function(k){locations[k]=gl.getUniformLocation(program,k);});resize();frame=requestAnimationFrame(render);}catch(error){console.warn("V3 WebGL fallback:",error);gl=null;}}
function initialise(target,options){canvas=target;readyCallback=options&&options.onReady;if(!canvas)return false;addEventListener("pointermove",function(event){if(model.lowMotion)return;model.pointerX=(event.clientX/innerWidth-.5)*2;model.pointerY=(event.clientY/innerHeight-.5)*-2;},{passive:true});addEventListener("resize",resize,{passive:true});if("IntersectionObserver" in window){observer=new IntersectionObserver(function(entries){model.visible=entries[0].isIntersecting;});observer.observe(canvas);}setTimeout(setup,32);return true;}
function resize(){if(!gl||!canvas)return;var rect=canvas.getBoundingClientRect(),embedded=innerWidth<1000,ratio=Math.min(devicePixelRatio||1,embedded?1:.82),width=Math.max(1,Math.floor(rect.width*ratio)),height=Math.max(1,Math.floor(rect.height*ratio));if(canvas.width!==width||canvas.height!==height){canvas.width=width;canvas.height=height;gl.viewport(0,0,width,height);}}
function camera(){var yaw=-.56+model.pointerX*.055+model.explode*.12,pitch=-.14+model.pointerY*.035-model.explode*.04,ro=[4.7+model.explode*1.1,3.2+model.explode*.55,6.8+model.explode*1.5];function rotate(pair,a){var c=Math.cos(a),s=Math.sin(a);return[c*pair[0]+s*pair[1],-s*pair[0]+c*pair[1]];}var xz=rotate([ro[0],ro[2]],yaw);ro[0]=xz[0];ro[2]=xz[1];var yz=rotate([ro[1],ro[2]],pitch);ro[1]=yz[0];ro[2]=yz[1];function norm(v){var l=Math.hypot(v[0],v[1],v[2])||1;return[v[0]/l,v[1]/l,v[2]/l];}function cross(a,b){return[a[1]*b[2]-a[2]*b[1],a[2]*b[0]-a[0]*b[2],a[0]*b[1]-a[1]*b[0]];}var ww=norm([-ro[0],.25-ro[1],-ro[2]]),uu=norm(cross(ww,[0,1,0])),vv=cross(uu,ww);return{ro:ro,ww:ww,uu:uu,vv:vv};}
function project(anchor,explodeVector){if(!canvas||!anchor)return null;var c=camera(),point=[anchor[0]+model.explode*((explodeVector&&explodeVector[0])||0),anchor[1]+model.explode*((explodeVector&&explodeVector[1])||0),anchor[2]+model.explode*((explodeVector&&explodeVector[2])||0)],v=[point[0]-c.ro[0],point[1]-c.ro[1],point[2]-c.ro[2]],dot=function(a,b){return a[0]*b[0]+a[1]*b[1]+a[2]*b[2];},depth=dot(v,c.ww);if(depth<=.01)return null;var u=2.55*dot(v,c.uu)/depth,w=2.55*dot(v,c.vv)/depth,rect=canvas.getBoundingClientRect();return{x:rect.left+(rect.width+u*rect.height)/2,y:rect.top+(rect.height-w*rect.height)/2,visible:u>-rect.width/rect.height-1&&u<rect.width/rect.height+1&&w>-2&&w<2};}
function render(now){frame=requestAnimationFrame(render);if(!gl||!model.visible)return;var cap=model.lowMotion?250:model.frameInterval;if(firstFrame&&now-last<cap)return;last=now;resize();var states={AUS:0,BEREIT:1,AUFHEIZEN:2,HALTEN:3,FEHLER:4},views={product:0,control:1,thermal:2,engineering:3,xray:4,signals:5},view=model.mode==="engineering"?3:(views[model.lens]||views[model.mode]||0),explodeTarget=model.mode==="engineering"?1:(model.lens==="xray"?.16:0);model.explode=model.lowMotion?explodeTarget:model.explode+(explodeTarget-model.explode)*.075;var heat=model.state==="AUFHEIZEN"||model.state==="HALTEN"?Math.max(.06,model.power/100):0;gl.useProgram(program);gl.uniform2f(locations.r,canvas.width,canvas.height);gl.uniform1f(locations.t,model.lowMotion?0:now/1000);gl.uniform1f(locations.power,model.power/100);gl.uniform1f(locations.temp,model.temperature);gl.uniform1f(locations.state,states[model.state]||0);gl.uniform1f(locations.explode,model.explode);gl.uniform1f(locations.thermal,heat);gl.uniform1f(locations.fan,model.lowMotion?0:Math.min(2,model.fan/2400));gl.uniform1f(locations.focus,{system:1,sensors:2,power:3}[model.focus]||1);gl.uniform1f(locations.viewMode,view);gl.uniform2f(locations.pointer,model.pointerX,model.pointerY);gl.drawArrays(gl.TRIANGLES,0,3);if(!firstFrame){firstFrame=true;document.body.classList.add("webgl-ready");if(readyCallback)readyCallback();}if(projectionCallback)projectionCallback(project);}
function update(status){model.state=status.state||"AUS";model.power=Number(status.power)||0;model.temperature=Number(status.temperature)||22;model.fan=Number(status.fan_rpm)||0;}
function setMode(mode){model.mode=mode||"product";}function setLens(lens){model.lens=lens||"product";}function setFocus(focus){model.focus=focus||"system";}function setLowMotion(value){model.lowMotion=!!value;}function setFrameRate(fps){model.frameInterval=1000/Math.max(1,Number(fps)||30);}function setProjectionListener(callback){projectionCallback=callback;if(callback&&canvas)requestAnimationFrame(function(){callback(project);});}
window.ProductScene={initialise:initialise,update:update,setMode:setMode,setLens:setLens,setFocus:setFocus,setLowMotion:setLowMotion,setFrameRate:setFrameRate,setProjectionListener:setProjectionListener,project:project,resize:resize};
})();
