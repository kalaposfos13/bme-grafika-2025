#include "framework.h"

const char* vertSource = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in vec3 cP;	// 0. bemeneti regiszter

	void main() {
		gl_Position = vec4(cP.x, cP.y, 1, 1); 	// bemenet m�r normaliz�lt eszk�zkoordin�t�kban
	}
)";

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

class Vec3 {
public:
    union {
        float x;
        float r;
    };
    union {
        float y;
        float g;
    };
    union {
        float z;
        float b;
    };
    Vec3(float f1, float f2, float f3) : x(f1), y(f2), z(f3) {}
    Vec3(float f1, float f2) : x(f1), y(f2), z(1.0f) {}
    Vec3(float f1) : x(f1), y(f1), z(f1) {}
    Vec3() : x(0), y(0), z(0) {}
    Vec3(const Vec3& o) : x(o.x), y(o.y), z(o.z) {}

    float len2() {
        return x * x + y * y;
    }
    float steepness() {
        return x / y;
    }
    Vec3 operator+(const Vec3& o) const {
        return Vec3(x + o.x, y + o.y, z + o.z);
    }
    Vec3 operator-(const Vec3& o) const {
        return Vec3(x - o.x, y - o.y, z - o.z);
    }
    Vec3 operator+=(const Vec3& o) {
        x += o.x, y += o.y, z += o.z;
        return *this;
    }
    Vec3 operator-=(const Vec3& o) {
        x -= o.x, y -= o.y, z -= o.z;
        return *this;
    }
    Vec3 operator*(const float& s) const {
        return Vec3(x * s, y * s, z * s);
    }
    Vec3 operator/(const float& s) const {
        return Vec3(x / s, y / s, z / s);
    }
    operator vec3() {
        return vec3(x, y, z);
    }
    Vec3 windowToViewSpace() {
        return Vec3((((float)x / winWidth) * 2.0f) - 1.0f,
                    (((float)(winHeight - y) / winHeight) * 2.0f) - 1.0f, 1.0f);
    }
};

enum class InputMode {
    AddPoint,
    AddLine,
    MoveLine,
    AddIntersectionPoint,
};

void getLineEndsFromTwoPoints(Vec3 p1, Vec3 p2, Vec3& op1, Vec3& op2) {
    Vec3 d = p1 - p2, o = d / (sqrt(d.x * d.x + d.y * d.y)) * 10.0f;
    op1 = p1 + o, op2 = p1 - o;
    // we get the direction vector, normalize it, multiply it by 3 so it will cover the largest
    // possible padding needed (diagonally across the screen), and add that offset in both ways to
    // the first vector
}
Vec3 getLineIntersection(Vec3 l1p1, Vec3 l1p2, Vec3 l2p1, Vec3 l2p2) {
    // Compute direction vectors
    Vec3 d1 = l1p2 - l1p1;
    Vec3 d2 = l2p2 - l2p1;
    
    // Check if lines are parallel
    if (d1.steepness() == d2.steepness()) {
        return Vec3(2, 2, 1); // Parallel case
    }
    
    // Solve for intersection using determinant method
    float det = d1.x * d2.y - d1.y * d2.x;
    if (det == 0) {
        return Vec3(2, 2, 1); // Lines are coincident or parallel
    }
    
    float t = ((l2p1.x - l1p1.x) * d2.y - (l2p1.y - l1p1.y) * d2.x) / det;
    
    // Compute intersection point
    Vec3 intersection = l1p1 + d1 * t;
    return Vec3(intersection.x, intersection.y, 1);
}


class GreenTriangleApp : public glApp {
    Geometry<Vec3>* pointList;
    Geometry<Vec3>* lineList;
    Vec3 lineTempPoint;
    GPUProgram* gpuProgram;
    InputMode mode;
    bool isMovingLine = false;
    Vec3 movingLineOrigin, movingLineCurrent;
    int movingLineIndex;

public:
    GreenTriangleApp() : glApp("Homework 1") {}

    // Inicializ�ci�,
    void onInitialization() {
        pointList = new Geometry<Vec3>;
        lineList = new Geometry<Vec3>;
        pointList->Vtx() = {};
        lineList->Vtx() = {};
        lineTempPoint = Vec3(0.0f, 0.0f, -1.0f);
        pointList->updateGPU();
        gpuProgram = new GPUProgram(vertSource, fragSource);
        glPointSize(10.0f);
        glLineWidth(3.0f);
    }

    void onDisplay() {
        glClearColor(0.125f, 0.125f, 0.125f, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glViewport(0, 0, winWidth, winHeight);

        Vec3 deltaLineMovement = ((movingLineCurrent - movingLineOrigin));
        if (isMovingLine) {
            lineList->Vtx()[movingLineIndex * 2] += deltaLineMovement;
            lineList->Vtx()[movingLineIndex * 2 + 1] += deltaLineMovement;
        }
        lineList->updateGPU();
        lineList->Draw(gpuProgram, GL_LINES, vec3(0.0f, 1.0f, 1.0f));
        if (isMovingLine) {
            lineList->Vtx()[movingLineIndex * 2] -= deltaLineMovement;
            lineList->Vtx()[movingLineIndex * 2 + 1] -= deltaLineMovement;
        }
        pointList->updateGPU();
        pointList->Draw(gpuProgram, GL_POINTS, vec3(1.0f, 0.0f, 0.0f));
    }

    void onMousePressed(MouseButton but, int pX, int pY) {
        if (but != MOUSE_LEFT) {
            return;
        }
        Vec3 mouseCoordsToViewCoords = Vec3(pX, pY, 1.0f).windowToViewSpace();

        switch (mode) {
        case InputMode::AddPoint:
            pointList->Vtx().push_back(mouseCoordsToViewCoords);
            pointList->updateGPU();
            printf("Added point\n");
            break;
        case InputMode::AddLine: {
            Vec3 selectedPoint = Vec3(0.0f, 0.0f, -1.0f);
            for (auto& pt : pointList->Vtx()) {
                // printf("pt: %f %f %f\n", pt.x, pt.y, pt.z);
                // printf("dist: %f\n", len2(mouseCoordsToViewCoords, pt));
                if (Vec3(mouseCoordsToViewCoords - pt).len2() < 0.005f) {
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
                Vec3 line1(1), line2(1);
                getLineEndsFromTwoPoints(selectedPoint, lineTempPoint, line1, line2);
                lineList->Vtx().push_back(line1);
                lineList->Vtx().push_back(line2);

                lineTempPoint.z = -1.0f;
                printf("Added line\n");
            }
            break;
        }
        case InputMode::MoveLine:
            isMovingLine = true;
            movingLineOrigin = Vec3(pX, pY).windowToViewSpace();
            movingLineCurrent = movingLineOrigin;
            movingLineIndex = 0;
            break;
        case InputMode::AddIntersectionPoint: {
            auto& ll = lineList->Vtx();
            pointList->Vtx().push_back(getLineIntersection(ll[0], ll[1], ll[2], ll[3]));
            break;
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

    virtual void onMouseReleased(MouseButton but, int pX, int pY) {
        if (!isMovingLine) {
            return;
        }
        isMovingLine = false;
        Vec3 deltaLineMovement = (movingLineCurrent - movingLineOrigin);
        lineList->Vtx()[movingLineIndex * 2] += deltaLineMovement;
        lineList->Vtx()[movingLineIndex * 2 + 1] += deltaLineMovement;
        refreshScreen();
    }
    virtual void onMouseMotion(int pX, int pY) {
        if (!isMovingLine) {
            return;
        }
        movingLineCurrent = Vec3(pX, pY).windowToViewSpace();
        refreshScreen();
    }
};

GreenTriangleApp app;
