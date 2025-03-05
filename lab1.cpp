//=============================================================================================
// Z�ld h�romsz�g: A framework.h oszt�lyait felhaszn�l� megold�s
//=============================================================================================
#include "framework.h"

// cs�cspont �rnyal�
const char* vertSource = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in vec3 cP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = vec4(cP.x, cP.y, 1, 1); 	// bemenet m�r normaliz�lt eszk�zkoordin�t�kban
	}
)";

// pixel �rnyal�
const char* fragSource = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans sz�n
	out vec4 fragmentColor;		// pixel sz�n

	void main() {
		fragmentColor = vec4(color.rgb, 1); // RGB -> RGBA
	}
)";

const int winWidth = 600, winHeight = 600;

enum class InputMode {
    AddPoint,
    AddLine,
    MoveLine,
    AddIntersectionPoint,
};
float len2(vec3 a, vec3 b) {
    return (a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y);
}

void getLineEndsFromTwoPoints(vec3 p1, vec3 p2, vec3& op1, vec3& op2) {
    vec3 d = p1 - p2, o = d / (sqrt(d.x * d.x + d.y * d.y)) * 3.0f;
    op1 = p1 + o, op2 = p1 - o;
    // we get the direction vector, normalize it, multiply it by 3 so it will cover the largest
    // possible padding needed (diagonally across the screen), and add that offset in both ways to
    // the first vector
}

class GreenTriangleApp : public glApp {
    Geometry<vec3>* pointList; // geometria
    Geometry<vec3>* lineList;  // geometria
    vec3 lineTempPoint;
    GPUProgram* gpuProgram; // cs�cspont �s pixel �rnyal�k
    InputMode mode;

public:
    GreenTriangleApp() : glApp("Homework 1") {}

    // Inicializ�ci�,
    void onInitialization() {
        pointList = new Geometry<vec3>;
        lineList = new Geometry<vec3>;
        pointList->Vtx() = {};
        lineList->Vtx() = {};
        lineTempPoint = vec3(0.0f, 0.0f, -1.0f);
        pointList->updateGPU();
        gpuProgram = new GPUProgram(vertSource, fragSource);
        glPointSize(10.0f);
        glLineWidth(3.0f);
    }

    // Ablak �jrarajzol�s
    void onDisplay() {
        glClearColor(0.125f, 0.125f, 0.125f, 0); // h�tt�r sz�n
        glClear(GL_COLOR_BUFFER_BIT);            // rasztert�r t�rl�s
        glViewport(0, 0, winWidth, winHeight);

        lineList->updateGPU();
        lineList->Draw(gpuProgram, GL_LINES, vec3(0.0f, 1.0f, 1.0f));
        pointList->updateGPU();
        pointList->Draw(gpuProgram, GL_POINTS, vec3(1.0f, 0.0f, 0.0f));
    }

    void onMousePressed(MouseButton but, int pX, int pY) {
        if (but != MOUSE_LEFT) {
            return;
        }
        vec3 mouseCoordsToViewCoords =
            vec3((((float)pX / winWidth) * 2.0f) - 1.0f,
                 (((float)(winHeight - pY) / winHeight) * 2.0f) - 1.0f, 1.0f);
        switch (mode) {
        case InputMode::AddPoint:
            pointList->Vtx().push_back(mouseCoordsToViewCoords);
            pointList->updateGPU();
            printf("Added point\n");
            break;
        case InputMode::AddLine: {
            vec3 selectedPoint = vec3(0.0f, 0.0f, -1.0f);
            for (auto& pt : pointList->Vtx()) {
                // printf("pt: %f %f %f\n", pt.x, pt.y, pt.z);
                // printf("dist: %f\n", len2(mouseCoordsToViewCoords, pt));
                if (len2(mouseCoordsToViewCoords, pt) < 0.005f) {
                    // printf("Clicked in an epsilon range of a point\n");
                    selectedPoint = pt;
                    break;
                }
            }
            if (selectedPoint.z == -1.0f) {
                return;
            } else if (lineTempPoint.z == -1) {
                lineTempPoint = selectedPoint;
                printf("Selected first point of line\n");
            } else {
                vec3 line1(1), line2(1);
                getLineEndsFromTwoPoints(selectedPoint, lineTempPoint, line1, line2);
                lineList->Vtx().push_back(line1);
                lineList->Vtx().push_back(line2);

                lineTempPoint.z = -1.0f;
                printf("Added line\n");
            }
        }
        default:
            break;
        }
        refreshScreen();
    }
    void onKeyboard(int key) {
        switch (key) {
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
