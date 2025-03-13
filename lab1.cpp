#include"framework.h"
char*c="#version 330\nin vec3 P;void main(){gl_Position=vec4(P,1);}";
char*f="#version 330\nuniform vec3 color;out vec4 o;void main(){o=vec4(color,1);}";
float t=1/3.,h=.5,q=.125;
vec3 v(float x,float y=t){return vec3(x,y,1);}
class A:public glApp{Geometry<vec3>*p,*l;GPUProgram*g;public:A():glApp(""){}
void onInitialization(){p=new Geometry<vec3>;l=new Geometry<vec3>;
p->Vtx()={v(-h,h),v(-t/2,0.4),v(t/5),v(-h,-h),v(h,-h)};l->Vtx()={v(-1,0.65),v(1,0.05),v(-1),v(1)};
p->updateGPU();l->updateGPU();g=new GPUProgram(c,f);
glPointSize(10);glLineWidth(3);glViewport(0,0,600,600);
glClearColor(q,q,q,0);}
void onDisplay(){glClear(16384);l->Draw(g,1,v(0,1));p->Draw(g,0,vec3(1,0,0));}};A a;