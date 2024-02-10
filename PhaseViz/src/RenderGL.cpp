#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
const double M_PI = 3.141592653589793238462643;
#elif defined __unix__
#include <unistd.h>
#define Sleep(a) usleep((a)*1000)
#endif

#include "RenderGL.h"
#include "OrbitGenerator.h"
#include "utils.h"


#include <chrono>
#include <GL/glew.h>
#include <GL/glut.h>
#include <fstream>
#include <glm/ext.hpp>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <memory>

GLuint loadAndCompile(std::string const &shaderFile, GLenum shader_type)
{
    GLuint shaderID = glCreateShader(shader_type);

    // Read the Shader code from the file
    std::string shaderCode;
    std::ifstream shaderStream(shaderFile, std::ios::in);
    if (shaderStream.is_open()) {
        std::stringstream sstr;
        sstr << shaderStream.rdbuf();
        shaderCode = sstr.str();
        shaderStream.close();
    } else {
        std::cout << "Unable to open " << shaderFile << std::endl;
        return 0;
    }

    GLint result = GL_FALSE;
    int infoLogLength;

    // Compile Shader
    std::cout << "Compiling shader : " << shaderFile << std::endl;
    char const *sourcePointer = shaderCode.c_str();
    glShaderSource(shaderID, 1, &sourcePointer, NULL);
    glCompileShader(shaderID);

    // Check Shader
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> shaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(shaderID, infoLogLength, NULL,
            &shaderErrorMessage[0]);
        std::cout << &shaderErrorMessage[0] << std::endl;
    }

    return shaderID;
}

GLuint loadShaders(std::string const &vertexFilePath,
    std::string const &fragmentFilePath)
{
    // Create the shaders
    GLuint vertexShaderID = loadAndCompile(vertexFilePath, GL_VERTEX_SHADER);
    GLuint fragmentShaderID = loadAndCompile(fragmentFilePath, GL_FRAGMENT_SHADER);

    // Link the program
    std::cout << "Linking program" << std::endl;
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    GLint result = GL_FALSE;
    int infoLogLength;
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL,
            &programErrorMessage[0]);
        std::cout << &programErrorMessage[0] << std::endl;
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
}

int coloredBody = 0;

std::shared_ptr<RenderGL> phaseRender;
Axis drawnAxis = Axis::POS0;
OrbitGenerator orbGen;

void initGLRendering(int argc, char **argv)
{
    glutInit(&argc, argv);
    phaseRender = std::make_shared<RenderGL>();
    orbGen.generateData();

    auto colors = orbGen.computeColors();
    std::cout << "Sending data to renderer." << std::endl;
    phaseRender->updateData(orbGen.orbitLines(), colors);
//    phaseRender->setProjAxes(solver.projectionAxes(drawnAxis));
}

void cdisplay(void)
{
    phaseRender->display();
}

void creshape(int w, int h)
{
    phaseRender->reshape(w, h);
}

void cmouseDrag(int x, int y)
{
    phaseRender->mouseDrag(x,y);
}

void cmouseClick(int button, int state, int x, int y)
{
    phaseRender->mouseClick(button, state, x, y);
}

void ckeyPressed(unsigned char key, int a, int b)
{
    phaseRender->keyPressed(key, a, b);
}

RenderGL::RenderGL() :
    numPoints(0),
    numLines(0),
    vboId(0),
    iboId(0),
    shaderId(0),
    eye(-0.3, 0.5, 5.0),
    modelMat(glm::mat4(1.0f)),
    modelMatInv(glm::mat4(1.0f)),
    modelViewMatInv(glm::mat4(1.0f)),
    modelViewProjMat(glm::mat4(1.0f)),
    projMat(glm::mat4(1.0f)),
    viewMat(glm::mat4(1.0f)),
    projAxes(0)
{

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

    // set window size
    glutInitWindowSize(700, 500);
    // set window location
    glutInitWindowPosition(250, 50);

    // create window with window text
    glutCreateWindow("Phase Space Visualizer");
    glewInit();

    // set background color to Black
    glClearColor(82/255.0f, 88/255.0f, 91/255.0f, 1.0);
    // set shade model to Flat
    glShadeModel(GL_FLAT);

    glEnable(GL_DEPTH_TEST);
    glDepthRange(0.1, 2000.0);

    shaderId = loadShaders("rotate.vert", "phase.frag");
    //   shaderId = loadShaders("plain.vert", "plain.frag");

    glutDisplayFunc(cdisplay);
    //  glutIdleFunc(updateRender);
    glutReshapeFunc(creshape);
    glutMotionFunc(cmouseDrag);
    glutMouseFunc(cmouseClick);
    glutKeyboardFunc(ckeyPressed);
}

RenderGL::~RenderGL()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);
    glDeleteProgram(shaderId);
}


void RenderGL::mouseDrag(int x, int y)
{
    int dx = x - dragPrevX;
    int dy = y - dragPrevY;
    dragPrevX = x;
    dragPrevY = y;

    float rotX = static_cast<float>(dx * M_PI / 180.0);
    float rotY = static_cast<float>(dy * M_PI / 180.0);
    modelMat = glm::rotate(
        modelMat, rotX, glm::vec3(modelMatInv * glm::vec4(0.0, 1.0, 0.0, 0.0)));
    modelMat = glm::rotate(
        modelMat, rotY, glm::vec3(modelMatInv * glm::vec4(1.0, 0.0, 0.0, 0.0)));
    modelMatInv = glm::inverse(modelMat);
    modelViewMatInv = glm::inverse(viewMat * modelMat);
    modelViewProjMat = projMat * viewMat * modelMat;

    glutPostRedisplay();

}

void RenderGL::mouseClick(int button, int state, int x, int y)
{
    if (state == GLUT_UP) return;  // Disregard redundant GLUT_UP events
    if (button == 0) {
        // Drag started.
        dragPrevX = x;
        dragPrevY = y;
    } else if (button == 3) {
        // Wheel reports as button 3(scroll up) and button 4(scroll down)
        moveForward();
    } else if (button == 4) {
        moveBackward();
    }
}

void RenderGL::keyPressed(unsigned char key, int a, int b)
{
    if (key == 'r') {
        orbGen.generateData();
		auto colors = orbGen.computeColors();
		std::cout << "Sending data to renderer." << std::endl;
		phaseRender->updateData(orbGen.orbitLines(), colors);
	}
    else if (key == 'w') {
        moveForward();
    } else if (key == 's') {
        moveBackward();
    } else if (key == 'a') {
        drawnAxis = (Axis)((drawnAxis+1)%AXIS_NELEMS);
        //setProjAxes(solver.projectionAxes(drawnAxis));
        glutPostRedisplay();
    } else if (key == 'c') {
        coloredBody = (coloredBody + 1)%3;
//        updateColors();
    }
}

void RenderGL::update()
{
    // Update vertex positions
    //solv3.advanceStep(0.1);

    glutPostRedisplay();
}

//void RenderGL::display()
//{
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    glUseProgram(shaderId);
//    GLuint mvpId = glGetUniformLocation(shaderId, "modelViewProjMatrix");
//    glUniformMatrix4fv(mvpId, 1, GL_FALSE, &modelViewProjMat[0][0]);
//
//    glBegin(GL_TRIANGLES);
//    for (int body = 0; body < 3; body++) {
//        glColor3f(body == 0, body == 1, body == 2);
//        glVertex3f(solv3.bodies[body].position[0] - 0.1, solv3.bodies[body].position[1] - 0.1,
//            solv3.bodies[body].position[2]);
//        glVertex3f(solv3.bodies[body].position[0] + 0.1, solv3.bodies[body].position[1] - 0.1,
//            solv3.bodies[body].position[2]);
//        glVertex3f(solv3.bodies[body].position[0], solv3.bodies[body].position[1] + 0.1,
//            solv3.bodies[body].position[2]);
//    }
//
//    glColor3f(0.5, 0.5, 0.5);
//    glVertex3f(-1, -1, 0);
//    glVertex3f( 1, -1, 0);
//    glVertex3f( 1,  1, 0);
//
//    glVertex3f( 1,  1, 0);
//    glVertex3f(-1,  1, 0);
//    glVertex3f(-1, -1, 0);
//    glEnd();
//    glutSwapBuffers();
//}
//

void renderString(float x, float y, void *font, const char *text, glm::vec3 const &rgb)
{
    glColor3f(rgb.r, rgb.g, rgb.b);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glRasterPos2f(x, y);

    auto c = text;
    while (*c != '\0') {
        glutBitmapCharacter(font, *c++);
    }
}

void RenderGL::display()
{
	// std::cout << "RenderGL::display. Num points:" << numPoints;
	// std::cout << "  Num lines:" << numLines << std::endl;
    // clearing the window or remove all drawn objects
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf(&projMat[0][0]);
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf(&modelMat[0][0]);
    glMultMatrixf(&viewMat[0][0]);

    glLineWidth(3.0f);
    glBegin(GL_LINES);
        glColor3f(1, 0, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(projAxes[0][0], projAxes[0][1], projAxes[0][2]);

        glColor3f(0, 1, 0);
        glVertex3f(0, 0, 0);
        glVertex3f(projAxes[1][0], projAxes[1][1], projAxes[1][2]);

        glColor3f(0, 0, 1);
        glVertex3f(0, 0, 0);
        glVertex3f(projAxes[2][0], projAxes[2][1], projAxes[2][2]);
    glEnd();

    glUseProgram(shaderId);
    GLuint mvpId = glGetUniformLocation(shaderId, "modelViewProjMatrix");
    glUniformMatrix4fv(mvpId, 1, GL_FALSE, &modelViewProjMat[0][0]);

    // bind VBOs before drawing
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    // enable vertex arrays
    glEnableVertexAttribArray(0);          // activate vertex position array
    glEnableVertexAttribArray(1);          // activate vertex color array

    size_t cOffset = sizeof(float) * numPoints * 3;

    // specify vertex arrays with their offsets
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void *)cOffset);

    // finally draw a cube with glDrawElements()
    glLineWidth(2.0);
    for (unsigned int i = 0; i < numLines; ++i) {
        int indexOffset = 4 * i * numPoints / numLines;
        glDrawElements(GL_LINE_STRIP,
            numPoints/numLines,
            GL_UNSIGNED_INT,         // data type
            (void *)static_cast<intptr_t>(indexOffset));         // offset to
    }
    // disable vertex arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // unbind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glUseProgram(0);

    renderString(0.8, -0.9, GLUT_BITMAP_TIMES_ROMAN_10, "HELLO!", glm::vec3(1,0.7,0));

    glutSwapBuffers();
}

void RenderGL::reshape(int width, int height)
{
    std::cout << "Reshape called. width=" << width << " height=" << height
        << std::endl;
    // adjusts the pixel rectangle for drawing to be the entire new window
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    projMat = glm::perspective((float)(5.0 * M_PI / 180),
        (GLfloat)width / (GLfloat)height, 0.1f, 2000.0f);
    viewMat = glm::lookAt(eye, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    modelViewProjMat = projMat * viewMat * modelMat;
}


float timeMultiplier()
{
    static auto prevTime = std::chrono::high_resolution_clock::now();
    static float multiplier = 0.1f;
    auto curTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - prevTime).count();
    prevTime = curTime;
    if (duration < 100) {
        multiplier *= 1.05f;
        multiplier = multiplier > 10.0f ? 10.0f : multiplier;
    } else
        multiplier = 1.0f;
    return multiplier;
}

void RenderGL::moveForward()
{
    float step = timeMultiplier();
    eye += glm::vec3(0, 0, -step);
    //    eye = eye + glm::vec3(modelViewMatInv * glm::vec4(0, 0, step, 1.0));
    viewMat = glm::lookAt(eye, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    modelViewMatInv = glm::inverse(viewMat * modelMat);
    modelViewProjMat = projMat * viewMat * modelMat;
//    display();
    glutPostRedisplay();
}

void RenderGL::moveBackward()
{
    float step = timeMultiplier();
    eye += glm::vec3(0, 0, step);
    //    eye = eye - glm::vec3(modelViewMatInv * glm::vec4(0, 0, step, 1.0));
    viewMat = glm::lookAt(eye, glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
    modelViewMatInv = glm::inverse(viewMat * modelMat);
    modelViewProjMat = projMat * viewMat * modelMat;
//    display();
    glutPostRedisplay();
}

void RenderGL::updateData(std::vector<std::vector<float>> const &lines,
    std::vector<std::vector<float>> const &colors)
{
	// std::cout << "updateData: numLines=" << lines.size() << std::endl;

    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);

    // create VBOs
    glGenBuffersARB(1, &vboId);  // for vertex buffer
    glGenBuffers(1, &iboId);     // for index buffer

    numPoints = static_cast<int>(lines.size() * lines[0].size() / 3);
    numLines = static_cast<int>(lines.size());

    std::vector<float> vertices;
    std::vector<float> vertColors;
    std::vector<unsigned int> indices;

    indices.resize(numPoints);

    for (auto line : lines) {
        vertices.insert(vertices.end(), std::make_move_iterator(line.begin()),
            std::make_move_iterator(line.end()));
    }
    for (auto lineColor : colors) {
        vertColors.insert(vertColors.end(),
            std::make_move_iterator(lineColor.begin()),
            std::make_move_iterator(lineColor.end()));
    }
    // std::cout << "== Stats:" << std::endl;
    // std::cout << "==              Num. lines:" << lines.size() << std::endl;
    // std::cout << "==    Num. verts per lines:" << lines[0].size() << std::endl;
    // std::cout << "==          Total vertices:" << numPoints << std::endl;
    // std::cout << "==              Total size:"
    //     << numPoints * 3 * sizeof(float) / 1024 << " kB" << std::endl;

    unsigned int curVertexIndex = 0;
    for (unsigned int curLine = 0; curLine < numLines; ++curLine) {

        size_t curLineVerts = lines[curLine].size() / 3;
        for (unsigned int i = 0; i < curLineVerts; ++i) {
            indices[curVertexIndex] = curVertexIndex;
            curVertexIndex++;
        }
    }

    std::cout << "== Normals computed. Sending data to the GPU." << std::endl;

    // copy vertex attribs data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    // reserve space
    glBufferData(
        GL_ARRAY_BUFFER,
        sizeof(float) * (vertices.size() + vertColors.size()), 0,
        GL_STATIC_DRAW);
    // copy positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size(),
        &vertices[0]);
    // copy colors
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(),
        sizeof(float) * vertColors.size(), &vertColors[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(),
        &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glutPostRedisplay();
}

void RenderGL::setProjAxes(glm::mat3 const &axes)
{
    //std::cout << "Axes:" << std::endl <<
    //    "[" << std::setprecision(3) << axes[0][0] << ", " << axes[0][1] << ", " << axes[0][2] << "]" << std::endl <<
    //    "[" << std::setprecision(3) << axes[1][0] << ", " << axes[1][1] << ", " << axes[1][2] << "]" << std::endl <<
    //    "[" << std::setprecision(3) << axes[2][0] << ", " << axes[2][1] << ", " << axes[2][2] << "]" << std::endl;
    projAxes = glm::mat3(glm::scale(glm::mat4(axes), glm::vec3(0.1f)));
}
