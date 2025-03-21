#include "framework.h"

// class vec2 {
//     float x, y;
//     vec2(float _x = 0,  float _y = 0) : x(_x), y(_y) {}
// };
// I love copy pasting things just to change the visibility of one thing
class GpuProgram {
    GLuint shaderProgramId = 0;
    bool waitError = true;

    bool checkShader(unsigned int shader, std::string message) { // shader fordítási hibák kezelése
        GLint infoLogLength = 0, result = 0;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (!result) {
            std::string errorMessage(infoLogLength, '\0');
            glGetShaderInfoLog(shader, infoLogLength, NULL, (GLchar*)errorMessage.data());
            printf("%s! \n Log: \n%s\n", message.c_str(), errorMessage.c_str());
            if (waitError)
                getchar();
            return false;
        }
        return true;
    }

    bool checkLinking(unsigned int program) { // shader szerkesztési hibák kezelése
        GLint infoLogLength = 0, result = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &result);
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        if (!result) {
            std::string errorMessage(infoLogLength, '\0');
            glGetProgramInfoLog(program, infoLogLength, nullptr, (GLchar*)errorMessage.data());
            printf("Failed to link shader program! \n Log: \n%s\n", errorMessage.c_str());
            if (waitError)
                getchar();
            return false;
        }
        return true;
    }

#ifdef FILE_OPERATIONS
    std::string file2string(const fs::path& _fileName) {
        std::string shaderCodeOut = "";
        std::ifstream shaderStream(_fileName);
        if (!shaderStream.is_open()) {
            printf("Error while opening shader code file %s!", _fileName.string().c_str());
            return "";
        }
        std::string line = "";
        while (std::getline(shaderStream, line)) {
            shaderCodeOut += line + "\n";
        }
        shaderStream.close();
        return shaderCodeOut;
    }
#endif

    std::string shaderType2string(GLenum shadeType) {
        switch (shadeType) {
        case GL_VERTEX_SHADER:
            return "Vertex";
            break;
        case GL_FRAGMENT_SHADER:
            return "Fragment";
            break;
        case GL_GEOMETRY_SHADER:
            return "Geometry";
            break;
        case GL_TESS_CONTROL_SHADER:
            return "Tessellation control";
            break;
        case GL_TESS_EVALUATION_SHADER:
            return "Tessellation evaluation";
            break;
        case GL_COMPUTE_SHADER:
            return "Compute";
            break;
        default:
            return "Unknown [shader type]";
            break;
        }
    }

public:
    GpuProgram() {}
    GpuProgram(const char* const vertexShaderSource, const char* const fragmentShaderSource,
               const char* const geometryShaderSource = nullptr) {
        create(vertexShaderSource, fragmentShaderSource, geometryShaderSource);
    }

    // this should be public
    int getLocation(const std::string& name) { // uniform változó címének lekérdezése
        int location = glGetUniformLocation(shaderProgramId, name.c_str());
        if (location < 0)
            printf("uniform %s cannot be set\n", name.c_str());
        return location;
    }

    void create(const char* const vertexShaderSource, const char* const fragmentShaderSource,
                const char* const geometryShaderSource = nullptr) {
        // Program létrehozása a forrás sztringbõl
        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        if (!vertexShader) {
            printf("Error in vertex shader creation\n");
            exit(1);
        }
        glShaderSource(vertexShader, 1, (const GLchar**)&vertexShaderSource, NULL);
        glCompileShader(vertexShader);
        if (!checkShader(vertexShader, "Vertex shader error"))
            return;

        // Program létrehozása a forrás sztringbõl, ha van geometria árnyaló
        GLuint geometryShader = 0;
        if (geometryShaderSource != nullptr) {
            geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
            if (!geometryShader) {
                printf("Error in geometry shader creation\n");
                exit(1);
            }
            glShaderSource(geometryShader, 1, (const GLchar**)&geometryShaderSource, NULL);
            glCompileShader(geometryShader);
            if (!checkShader(geometryShader, "Geometry shader error"))
                return;
        }

        // Program létrehozása a forrás sztringbõl
        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        if (!fragmentShader) {
            printf("Error in fragment shader creation\n");
            exit(1);
        }

        glShaderSource(fragmentShader, 1, (const GLchar**)&fragmentShaderSource, NULL);
        glCompileShader(fragmentShader);
        if (!checkShader(fragmentShader, "Fragment shader error"))
            return;

        shaderProgramId = glCreateProgram();
        if (!shaderProgramId) {
            printf("Error in shader program creation\n");
            exit(-1);
        }
        glAttachShader(shaderProgramId, vertexShader);
        glAttachShader(shaderProgramId, fragmentShader);
        if (geometryShader > 0)
            glAttachShader(shaderProgramId, geometryShader);

        // Szerkesztés
        if (!link())
            return;

        // Ez fusson
        glUseProgram(shaderProgramId);
    }

#ifdef FILE_OPERATIONS
    bool addShader(const fs::path& _fileName) {
        GLenum shaderType = 0;
        auto ext = _fileName.extension();
        if (ext == ".vert") {
            shaderType = GL_VERTEX_SHADER;
        } else if (ext == ".frag") {
            shaderType = GL_FRAGMENT_SHADER;
        } else if (ext == ".geom") {
            shaderType = GL_GEOMETRY_SHADER;
        } else if (ext == ".tesc") {
            shaderType = GL_TESS_CONTROL_SHADER;
        } else if (ext == ".tese") {
            shaderType = GL_TESS_EVALUATION_SHADER;
        } else if (ext == ".comp") {
            shaderType = GL_COMPUTE_SHADER;
        } else {
            printf("Unknown shader extension");
            return false;
        }
        return addShader(shaderType, _fileName);
    }

    bool addShader(GLenum shaderType, const fs::path& _fileName) {
        std::string shaderCode = file2string(_fileName);
        GLuint shaderID = glCreateShader(shaderType);
        if (!shaderID) {
            printf("Error in vertex shader creation\n");
            exit(1);
        }
        const char* sourcePointer = shaderCode.data();
        GLint sourceLength = static_cast<GLint>(shaderCode.length());
        glShaderSource(shaderID, 1, &sourcePointer, &sourceLength);
        glCompileShader(shaderID);
        if (!checkShader(shaderID, shaderType2string(shaderType) + " shader error"))
            return false;
        if (shaderProgramId == 0)
            shaderProgramId = glCreateProgram();
        glAttachShader(shaderProgramId, shaderID);
        return true;
    }
#endif

    bool link() {
        glLinkProgram(shaderProgramId);
        return checkLinking(shaderProgramId);
    }

    void Use() {
        glUseProgram(shaderProgramId);
    } // make this program run

    void setUniform(int i, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniform1i(location, i);
    }

    void setUniform(float f, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniform1f(location, f);
    }

    void setUniform(const vec2& v, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniform2fv(location, 1, &v.x);
    }

    void setUniform(const vec3& v, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniform3fv(location, 1, &v.x);
    }

    void setUniform(const vec4& v, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniform4fv(location, 1, &v.x);
    }

    void setUniform(const mat4& mat, const std::string& name) {
        int location = getLocation(name);
        if (location >= 0)
            glUniformMatrix4fv(location, 1, GL_FALSE, &mat[0][0]);
    }
    operator GPUProgram() {
        return (GPUProgram) * this;
    }
    ~GpuProgram() {
        if (shaderProgramId > 0)
            glDeleteProgram(shaderProgramId);
    }
};

// csúcspont árnyaló
const char* vertSourceBezier = R"(
	#version 330				
    precision highp float;

	layout(location = 0) in float t;
    uniform vec2 controlPoints[4];

    vec2 lerp(vec2 a, vec2 b, float t) {
        return vec2((1.0f - t)*a.x + t*b.x, (1.0f - t)*a.y + t*b.y);
    }

	void main() {
        vec2 p1l1 = lerp(controlPoints[0], controlPoints[1], t);
        vec2 p1l2 = lerp(controlPoints[1], controlPoints[2], t);
        vec2 p1l3 = lerp(controlPoints[2], controlPoints[3], t);

        vec2 p2l1 = lerp(p1l1, p1l2, t);
        vec2 p2l2 = lerp(p1l2, p1l3, t);

        vec2 p3l1 = lerp(p2l1, p2l2, t);

		gl_Position = vec4(p3l1, 0, 1);
	}
)";
const char* vertSourceBezierDebug = R"(
    #version 330				
    precision highp float;

	layout(location = 0) in float t;
    uniform vec2 controlPoints[4];

	void main() {
		gl_Position = vec4(controlPoints[int(t*3)], 0, 1);
	}
)";
const char* vertSourcePoints = R"(
    #version 330
    precision highp float;

	layout(location = 0) in vec2 pt;
    void main() {
        gl_Position = vec4(pt, 0, 1);
    }
)";

// pixel árnyaló
const char* fragSourceColor = R"(
	#version 330
    precision highp float;

	uniform vec3 color;			// konstans szín
	out vec4 fragmentColor;		// pixel szín

	void main() {
		fragmentColor = vec4(color, 1); // RGB -> RGBA
	}
)";
enum InputMode {
    AddPoint,
    MovePoint,
};
const int winWidth = 600, winHeight = 600;
const int curveSegments = 4;
float curviness = 0.5;
vec2 windowToViewSpace(vec2 v) {
    return vec2((((float)v.x / winWidth) * 2.0f) - 1.0f,
                (((float)(winHeight - v.y) / winHeight) * 2.0f) - 1.0f);
}
class GreenTriangleApp : public glApp {
    Geometry<float>* segments;
    Geometry<vec2>*controlPoints, *splinePoints;
    GpuProgram* gpuProgramBezier;
    GpuProgram* gpuProgramPoints;

    InputMode mode = AddPoint;
    bool isMovingPoint = false, displayControlPoints = false, displaySplinePoints = true;
    vec2 movingPointOrigin, movingPointCurrent;
    int movingPointIndex;

public:
    GreenTriangleApp() : glApp("bezier") {}

    // Inicializáció,
    void onInitialization() {
        segments = new Geometry<float>;
        controlPoints = new Geometry<vec2>;
        splinePoints = new Geometry<vec2>;
        segments->Vtx().clear();
        for (int i = 0; i <= curveSegments; i++) {
            float t = (float)i / (float)curveSegments;
            segments->Vtx().emplace_back(t);
        }
        segments->updateGPU();
        splinePoints->Vtx() = {
            vec2(0, 0),
            vec2(0, 0),
        };
        gpuProgramBezier = new GpuProgram(vertSourceBezier, fragSourceColor);
        gpuProgramPoints = new GpuProgram(vertSourcePoints, fragSourceColor);
        glPointSize(10);
        glLineWidth(3);
    }

    // Ablak újrarajzolás
    void onDisplay() {
        glClearColor(0, 0, 0, 1);     // háttér szín
        glClear(GL_COLOR_BUFFER_BIT); // rasztertár törlés
        if (splinePoints->Vtx().size() < 4) {
            return;
        }
        splinePoints->Vtx()[movingPointIndex] += movingPointCurrent - movingPointOrigin;
        auto& sp = splinePoints->Vtx();
        int last = sp.size() - 1;
        // calculate pre start and post end point of spline buffer
        sp[0] = 2.0f * sp[1] - sp[2];
        sp[last] = 2.0f * sp[last - 1] - sp[last - 2];
        // draw splines
        for (int i = 1; i < splinePoints->Vtx().size() - 2; i++) {
            controlPoints->Vtx() = {
                sp[i],
                sp[i] + (sp[i + 1] - sp[i - 1]) * curviness / 3.0f,
                sp[i + 1] - (sp[i + 2] - sp[i]) * curviness / 3.0f,
                sp[i + 1],
            };
            // draw a single curve
            gpuProgramBezier->Use();
            glUniform2fv(gpuProgramBezier->getLocation("controlPoints"), 8,
                         (float*)controlPoints->Vtx().data());
            segments->updateGPU();
            segments->Draw((GPUProgram*)gpuProgramBezier, GL_LINE_STRIP, vec3(1.0f, 1.0f, 0.0f));
            // draw the control points for the current curve with alternating colors
            if (displayControlPoints) {
                glPointSize(10);
                gpuProgramPoints->Use();
                controlPoints->updateGPU();
                controlPoints->Draw((GPUProgram*)gpuProgramPoints, GL_POINTS,
                                    vec3(0.0f, i % 2 == 1 ? 1.0f : 0.0f, i % 2 == 0 ? 1.0f : 0.0f));
            }
        }
        // draw the spline points
        // glPointSize(5);
        if (displaySplinePoints) {
            gpuProgramPoints->Use();
            sp[0] = vec2(2, 2);
            sp[last] = vec2(2, 2);
            splinePoints->updateGPU();
            splinePoints->Draw((GPUProgram*)gpuProgramPoints, GL_POINTS, vec3(1.0f, 0.0f, 0.0f));
        }

        splinePoints->Vtx()[movingPointIndex] += -(movingPointCurrent - movingPointOrigin);
    }
    void onMousePressed(MouseButton but, int pX, int pY) {
        vec2 click = windowToViewSpace(vec2(pX, pY));
        if (but == MOUSE_LEFT) {
            if (mode == MovePoint) {
                float minDist = length(click - splinePoints->Vtx()[0]);
                int minIdx = 0;
                for (int i = 1; i < splinePoints->Vtx().size(); i++) {
                    float dist = length(click - splinePoints->Vtx()[i]);
                    if (dist < minDist) {
                        minDist = dist;
                        minIdx = i;
                    }
                }
                if (minDist <= 0.05) {
                    printf("we clicked on one of the control points\n");
                    isMovingPoint = true;
                    movingPointIndex = minIdx;
                    movingPointOrigin = click;
                    movingPointCurrent = click;
                }
            } else if (mode == AddPoint) {
                splinePoints->Vtx().insert(--splinePoints->Vtx().end(), click);
                printf("Added point %f %f\n", click.x, click.y);
                // *--splinePoints->Vtx().end() = click;
                // splinePoints->Vtx().push_back(vec2(0));
            }
        } else if (but == MOUSE_RIGHT) {
            float minDist = length(click - splinePoints->Vtx()[0]);
            int minIdx = 0;
            for (int i = 1; i < splinePoints->Vtx().size(); i++) {
                float dist = length(click - splinePoints->Vtx()[i]);
                if (dist < minDist) {
                    minDist = dist;
                    minIdx = i;
                }
            }
            if (minDist <= 0.05) {
                printf("we clicked on one of the control points\n");
                splinePoints->Vtx().erase(splinePoints->Vtx().begin() + minIdx);
            }
        }
        refreshScreen();
    }
    void onMouseReleased(MouseButton but, int pX, int pY) {
        if (!isMovingPoint) {
            return;
        }
        isMovingPoint = false;
        splinePoints->Vtx()[movingPointIndex] += movingPointCurrent - movingPointOrigin;
        movingPointCurrent = vec2(0, 0);
        movingPointOrigin = vec2(0, 0);
        refreshScreen();
    }
    void onMouseMotion(int pX, int pY) {
        if (!isMovingPoint) {
            return;
        }
        movingPointCurrent = windowToViewSpace(vec2(pX, pY));
        refreshScreen();
    }
    void onKeyboard(int key) {
        switch (key) {
        case 'p':
            mode = AddPoint;
            break;
        case 'm':
            mode = MovePoint;
            break;
        case 'q':
            exit(0);
        case 'i':
            curviness += 0.05;
            break;
        case 'k':
            curviness -= 0.05;
            break;
        case 'o':
            displayControlPoints ^= true;
            break;
        case 'l':
            displaySplinePoints ^= true;
            break;
        default:
            break;
        }
        refreshScreen();
    }
};

GreenTriangleApp app;
