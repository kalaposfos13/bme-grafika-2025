#include "framework.h"
const char* vertSource = R"(
	#version 330
	layout(location = 0) in vec3 cP;
	void main() {
		gl_Position = vec4(cP.x, cP.y, 1, 1);
	}
)";
const char* fragSource = R"(
	#version 330
	uniform vec3 color;
	out vec4 fragmentColor;
	void main() {
		fragmentColor = vec4(color.rgb, 1);
	}
)";
vec3 Vec3(float x, float y) {
    return vec3((((float)x / 600) * 2.0f) - 1.0f,
                (((float)(600 - y) / 600) * 2.0f) - 1.0f, 1.0f);
}
class GreenTriangleApp : public glApp {
    Geometry<vec3>* pointList;
    Geometry<vec3>* lineList;
    GPUProgram* gpuProgram;
public:
    GreenTriangleApp() : glApp("Homework 1 - code golfed") {}
    void onInitialization() {
        pointList = new Geometry<vec3>;
        lineList = new Geometry<vec3>;
        pointList->Vtx() = {Vec3(150, 150), Vec3(250, 180), Vec3(320, 200), Vec3(150, 450),
                            Vec3(450, 450)};
        lineList->Vtx() = {Vec3(0, 105), Vec3(600, 285), Vec3(0, 200), Vec3(600, 200)};
        pointList->updateGPU();
        lineList->updateGPU();
        gpuProgram = new GPUProgram(vertSource, fragSource);
        glPointSize(10.0f);
        glLineWidth(3.0f);
        glViewport(0, 0, 600, 600);
        glClearColor(0.125f, 0.125f, 0.125f, 0);
    }
    void onDisplay() {
        glClear(GL_COLOR_BUFFER_BIT);
        lineList->Draw(gpuProgram, GL_LINES, vec3(0.0f, 1.0f, 1.0f));
        pointList->Draw(gpuProgram, GL_POINTS, vec3(1.0f, 0.0f, 0.0f));
    }
};
GreenTriangleApp app;