//=============================================================================================
// Zöld háromszög: A framework.h osztályait felhasználó megoldás
//=============================================================================================
#include "framework.h"
#include <GLFW/glfw3.h>

constexpr float PI = 3.141591f;

// Csúcspont árnyaló
const char *vertSource = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in vec2 cP; // 0. bemeneti regiszter

	void main() {
		gl_Position = vec4(cP.x, cP.y, 0, 1); // Bemenet már normalizált eszközkoordinátákban
	}
)";

// Pixel árnyaló
const char *fragSource = R"(
	#version 330
    precision highp float;
	uniform vec2 window;
	
	uniform vec3 color;      // Konstans szín
	uniform float radius;
	uniform vec2 circleCenter;
	out vec4 fragmentColor;  // Pixel szín

	void main() {
		if (length(gl_FragCoord.xy/window - circleCenter.xy) > radius) {
			discard;
		} else {
			fragmentColor = vec4(color, 1); // RGB -> RGBA
		}
	}
)";

const int winWidth = 600, winHeight = 600;

class VasarelyAnimation : public glApp
{
	Geometry<vec2> *triangle; // Geometria
	GPUProgram *gpuProgram;	  // Csúcspont és pixel árnyalók
	glm::vec3 a{0.0f, 0.0f, 0.0f};
	float dT = 0;

public:
	VasarelyAnimation() : glApp("Vasarely animation") {}

	// Inicializáció
	void onInitialization()
	{
		triangle = new Geometry<vec2>;
		triangle->Vtx() = {vec2(-1.0f, -1.0f), vec2(-1.0f, 3.0f), vec2(3.0f, -1.0f)};
		triangle->updateGPU();
		gpuProgram = new GPUProgram(vertSource, fragSource);
		gpuProgram->setUniform(vec2((float)winWidth, (float)winHeight), "window");
		glViewport(0, 0, winWidth, winHeight);
	}

	// Ablak újrarajzolás
	void onDisplay()
	{
		glClearColor(0, 0, 100, 0);	  // Háttér szín
		glClear(GL_COLOR_BUFFER_BIT); // Rasztertár törlés
		int slices = 10;
		for (int i = 0; i < slices; i++)
		{
			float vertical_offset = (1 - cos(((float)i / slices - 0.5f) * PI)) / 10.0f * sin(dT * 2);
			float horizontal_offset = (1 - cos(((float)i / slices - 0.5f) * PI)) / 10.0f * cos(dT * 2);
			// float horizontal_offset = (cos(((float)i / slices - 0.5f) * PI + PI / 2.0f)) / 8.0f * sin(dT);
			gpuProgram->setUniform(vec2(0.5f + horizontal_offset, 0.5f + vertical_offset), "circleCenter");
			gpuProgram->setUniform((slices - i) / (float)slices / 2.2f, "radius");
			triangle->Draw(gpuProgram, GL_TRIANGLES, vec3((float)i / slices));
		}
	}

	void onTimeElapsed(float startTime, float endTime)
	{
		dT += (endTime - startTime);
		refreshScreen();
	}
	void onKeyboard(int key) {
		if (key == GLFW_KEY_SPACE) {
			exit(0);
		}
		printf("key: %d\n", key);
	}
};

VasarelyAnimation app;
