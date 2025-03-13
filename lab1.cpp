#include "framework.h"
const char* c=R"(#version 330
layout(location=0)in vec3 cP;void main(){gl_Position=vec4(cP,1);})";
const char* f=R"(#version 330
uniform vec3 color;out vec4 o;void main(){o=vec4(color,1);})";
float t=0.333333;
vec3 v(float x,float y){return vec3(x,y,1);}
class A:public glApp{Geometry<vec3>*p,*l;GPUProgram*g;public:A():glApp("HW1"){}
void onInitialization(){p=new Geometry<vec3>;l=new Geometry<vec3>;
p->Vtx()={v(-0.5,0.5),v(-0.166667,0.4),v(0.066667,t),v(-0.5,-0.5),v(0.5,-0.5)};
l->Vtx()={v(-1,0.65),v(1,0.05),v(-1,t),v(1,t)};
p->updateGPU();l->updateGPU();g=new GPUProgram(c,f);
glPointSize(10);glLineWidth(3);glViewport(0,0,600,600);
glClearColor(.125,.125,.125,0);}
void onDisplay(){glClear(16384);l->Draw(g,GL_LINES,v(0,1));p->Draw(g,GL_POINTS,vec3(1,0,0));}};A a;
