//=============================================================================================
// Z�ld h�romsz�g: A framework.h oszt�lyait felhaszn�l� megold�s
//=============================================================================================
#include "framework.h"

// cs�cspont �rnyal�
const char *vertSource = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in vec3 cP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = vec4(cP.x, cP.y, 1, 1); 	// bemenet m�r normaliz�lt eszk�zkoordin�t�kban
	}
)";

// pixel �rnyal�
const char *fragSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans sz�n
	out vec4 fragmentColor;		// pixel sz�n

	void main() {
		fragmentColor = vec4(color.rgb, 1); // RGB -> RGBA
	}
)";

const int winWidth = 600, winHeight = 600;

enum class InputMode
{
	AddPoint,
	AddLine,
	MoveLine,
	AddIntersectionPoint,
};

class GreenTriangleApp : public glApp
{
	Geometry<vec3> *pointList; // geometria
	GPUProgram *gpuProgram;	   // cs�cspont �s pixel �rnyal�k
	InputMode mode;

public:
	GreenTriangleApp() : glApp("Homework 1") {}

	// Inicializ�ci�,
	void onInitialization()
	{
		pointList = new Geometry<vec3>;
		pointList->Vtx() = {};
		pointList->updateGPU();
		gpuProgram = new GPUProgram(vertSource, fragSource);
		glPointSize(10.0f);
	}

	// Ablak �jrarajzol�s
	void onDisplay()
	{
		glClearColor(0.125f, 0.125f, 0.125f, 0); // h�tt�r sz�n
		glClear(GL_COLOR_BUFFER_BIT);			 // rasztert�r t�rl�s
		glViewport(0, 0, winWidth, winHeight);

		pointList->Draw(gpuProgram, GL_POINTS, vec3(1.0f, 0.0f, 0.0f));
	}

	void onMousePressed(MouseButton but, int pX, int pY)
	{
		if (but != MOUSE_LEFT) {
			return;
		}
		switch (mode)
		{
		case InputMode::AddPoint: {
			vec3 newPoint = vec3((((float)pX / winWidth) * 2.0f) - 1.0f, (((float)(winHeight - pY) / winHeight) * 2.0f) - 1.0f, 1.0f);
			pointList->Vtx().push_back(newPoint);
			pointList->updateGPU();
			break;
		}
		default:
			break;
		}
		refreshScreen();
	}
	void onKeyboard(int key)
	{
		switch (key)
		{
		case ' ':
			exit(0);
			break;
		case 'p':
			mode = InputMode::AddPoint;
			printf("mode = AddPoint\n");
			break;
		case 'l':
			mode = InputMode::AddLine;
			printf("mode = AddLine\n");
			break;
		case 'm':
			mode = InputMode::MoveLine;
			printf("mode = MoveLine\n");
			break;
		case 'i':
			mode = InputMode::AddIntersectionPoint;
			printf("mode = AddIntersectionPoint\n");
			break;
		default:
			break;
		}
	}
};

GreenTriangleApp app;
