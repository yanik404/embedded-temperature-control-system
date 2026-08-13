(function(){
"use strict";
var canvas,ctx,frame=0,last=0,particles=[],status={state:"AUS",power:0,fan_rpm:0},mode="overview",lowMotion=false,frameInterval=33;
function initialise(target){canvas=target;if(!canvas)return;ctx=canvas.getContext("2d");if(!ctx)return;for(var i=0;i<34;i++)particles.push(seed(i/34));addEventListener("resize",resize,{passive:true});resize();frame=requestAnimationFrame(draw);}
function seed(progress){return{x:(Math.random()-.5)*.33,y:progress||Math.random(),phase:Math.random()*6.28,speed:.0012+Math.random()*.0015};}
function resize(){if(!ctx)return;var rect=canvas.getBoundingClientRect(),ratio=Math.min(devicePixelRatio||1,1.25),w=Math.max(1,Math.floor(rect.width*ratio)),h=Math.max(1,Math.floor(rect.height*ratio));if(canvas.width!==w||canvas.height!==h){canvas.width=w;canvas.height=h;ctx.setTransform(ratio,0,0,ratio,0,0);}}
function draw(now){frame=requestAnimationFrame(draw);if(!ctx||now-last<(lowMotion?250:frameInterval))return;last=now;resize();var rect=canvas.getBoundingClientRect(),w=rect.width,h=rect.height,active=status.state==="AUFHEIZEN"||status.state==="HALTEN",intensity=active?Math.max(.07,Number(status.power||0)/100):0;ctx.clearRect(0,0,w,h);if(!intensity)return;
 var cx=w*.50,base=h*.81,top=h*.28;
 var field=ctx.createRadialGradient(cx,base,0,cx,base,h*.48);field.addColorStop(0,"rgba(255,102,35,"+(.22*intensity)+")");field.addColorStop(.32,"rgba(255,137,55,"+(.10*intensity)+")");field.addColorStop(1,"rgba(255,126,43,0)");ctx.fillStyle=field;ctx.fillRect(0,0,w,h);
 ctx.save();ctx.globalCompositeOperation="lighter";ctx.lineWidth=.7;
 particles.forEach(function(point,index){if(!lowMotion){point.y+=point.speed*(.45+intensity)*33;if(point.y>1)Object.assign(point,seed(0));}var y=base-(base-top)*point.y,x=cx+point.x*w+Math.sin(point.phase+now*.00055+index)*w*.018*point.y;var alpha=(1-point.y)*(.12+.34*intensity);ctx.beginPath();ctx.moveTo(x,y+9);ctx.quadraticCurveTo(x+Math.sin(point.phase)*8,y+3,x,y);ctx.strokeStyle="rgba(255,"+(105+Math.round(point.y*75))+",70,"+alpha+")";ctx.stroke();});
 if(mode==="thermal"){for(var ring=0;ring<4;ring++){var phase=(lowMotion?.45:(now*.00012+ring*.23)%1);ctx.beginPath();ctx.ellipse(cx,base-(h*.43*phase),w*(.09+.12*phase),h*(.018+.025*phase),0,0,Math.PI*2);ctx.strokeStyle="rgba(255,136,66,"+((1-phase)*.16*intensity)+")";ctx.stroke();}}
 var fan=Math.min(1,Number(status.fan_rpm||0)/2800);if(fan){ctx.strokeStyle="rgba(104,226,241,"+(.12*fan)+")";for(var a=0;a<3;a++){var yy=h*(.88+a*.035);ctx.beginPath();ctx.moveTo(w*.35,yy);ctx.bezierCurveTo(w*.42,yy+Math.sin(now*.002+a)*8,w*.58,yy-6,w*.67,yy-2);ctx.stroke();}}
 ctx.restore();}
function update(next){status=next||status;}
function setMode(next){mode=next||"overview";}
function setLowMotion(value){lowMotion=!!value;}
function setFrameRate(fps){frameInterval=1000/Math.max(1,Number(fps)||30);}
window.ThermalOverlay={initialise:initialise,update:update,setMode:setMode,setLowMotion:setLowMotion,setFrameRate:setFrameRate};
})();
