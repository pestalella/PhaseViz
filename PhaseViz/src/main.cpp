#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
const double M_PI = 3.141592653589793238462643;
#elif defined __unix__ 
#include <unistd.h>
#define Sleep(a) usleep((a)*1000)
#endif

#include "RenderGL.h"
#include "ThreeBodySolver.h"
#include "utils.h"

#include <algorithm>
#include <cmath>
#include <ctime>
#include <GL/glew.h>
#include <GL/glut.h>
#include <iomanip>
#include <iostream>
#include <memory>

std::shared_ptr<RenderGL> phaseRender;
ThreeBodySolver solver;
Axis drawnAxis = Axis::POS0;

ThreeBodySystem randomSystem()
{
    float radius = 0.2;
    Body body0 = {randomVector(radius), randomVector(0)};
    Body body1 = {randomVector(radius), randomVector(0)};
    Body body2 = {randomVector(radius), randomVector(0)};

    //const glm::vec3 bias1(20, 10, 0);
    //const glm::vec3 bias2(-10, 0, 0);
    //const glm::vec3 bias3(0, -10, 0);
    //body0.position += bias1;
    //body1.position += bias2;
    //body2.position += bias3;

    body0.position.z = 0;
    body1.position.z = 0;
    body2.position.z = 0;

    //    glm::dvec3 offset = 1/3.0*(body0.position + body1.position + body2.position);
    glm::dvec3 posOffset = body0.position;
    //  glm::dvec3 offset(0);
//    glm::dvec3 velocity = 1/3.0*(body0.velocity + body1.velocity + body2.velocity);
    //    glm::dvec3 velocity(0);

    // Center the system around the origin of coords
    body0.position -= posOffset;
    body1.position -= posOffset;
    body2.position -= posOffset;
    body1.position.y = -1.0;
    body2.position.y = 2.0;

    // Remove system velocity
    //body0.velocity -= velocity;
    //body1.velocity -= velocity;
    //body2.velocity -= velocity;

    return {body0, body1, body2};
}

bool tooClose(glm::vec3 candidate, std::vector<glm::vec3> const &pointCloud)
{
    float minDistance = 1E10;
    for (auto p: pointCloud) {
        auto dist = glm::distance(candidate, p);
        if (dist < 0.1) return true;
        minDistance = (std::min)(dist, minDistance);
    }
    return minDistance > 0.15;
}

std::vector<std::vector<ThreeBodySystem>> states;
std::vector<std::vector<float>> lines;
int coloredBody = 0;

std::vector<std::vector<float>> computeColors()
{
    std::vector<std::vector<float>> colors;
    for (auto curOrbit : states) {
        std::vector<float> orbitColor;
        for (auto state : curOrbit) {
            orbitColor.push_back(glm::length(state.body[coloredBody].velocity)*0.01);
            orbitColor.push_back(state.body[coloredBody].position.x*state.body[coloredBody].position.x*10.0);
            orbitColor.push_back(state.body[coloredBody].position.y*state.body[coloredBody].position.y*10.0);
        }
        colors.push_back(orbitColor);
    }
    return colors;
}

void generateData()
{
    std::vector<glm::vec3> startingPoints;

    int numLines = 1;

//    std::vector<std::vector<ThreeBodySystem>> states;
//    std::vector<std::vector<float>> lines;

    std::cout << "Generating data..." << std::endl;
    for (int curLine = 0; curLine < numLines; ++curLine) {
        auto tbs = randomSystem();
        auto orbits = solver.computeOrbit(tbs, 8000);
        std::cout << "line " << curLine << std::endl;
        states.push_back(orbits.first);
        lines.push_back(orbits.second);
    }

    //for (int curLine = 0; curLine < numLines; ++curLine) {
    //    size_t numVerts = lines[curLine].size()/3;
    //    for (unsigned int i = 0; i < numVerts; ++i) {
    //        lines[curLine][3*i + 0] -= center.x;
    //        lines[curLine][3*i + 1] -= center.y;
    //        lines[curLine][3*i + 2] -= center.z;
    //    }
    //}

    auto colors = computeColors();

    std::cout << "Sending data to renderer." << std::endl;
    phaseRender->updateData(lines, colors);
    phaseRender->setProjAxes(solver.projectionAxes(drawnAxis));
}

void updateColors()
{
    auto colors = computeColors();
    std::cout << "Sending data to renderer." << std::endl;
    phaseRender->updateData(lines, colors);
}

void updateRender(void)
{
    Sleep(100);
    phaseRender->update();
}

void display(void)
{
    phaseRender->display();
}

void reshape(int w, int h)
{
    phaseRender->reshape(w, h);
}

void initGLRendering()
{
    // set background color to Black
    glClearColor(82/255.0f, 88/255.0f, 91/255.0f, 1.0);
    // set shade model to Flat
    glShadeModel(GL_FLAT);

    glEnable(GL_DEPTH_TEST);
    glDepthRange(0.1, 2000.0);

    phaseRender = std::make_shared<RenderGL>();
}

int dragPrevX = 0, dragPrevY = 0;

void mouseDrag(int x, int y)
{
    int offsetX = x - dragPrevX;
    int offsetY = y - dragPrevY;
    dragPrevX = x;
    dragPrevY = y;
    phaseRender->mouseDrag(offsetX, offsetY);
}

void mouseClick(int button, int state, int x, int y)
{
    if (state == GLUT_UP) return;  // Disregard redundant GLUT_UP events
    if (button == 0) {
        // Drag started.
        dragPrevX = x;
        dragPrevY = y;
    } else if (button == 3) {
        // Wheel reports as button 3(scroll up) and button 4(scroll down)
        phaseRender->moveForward();
    } else if (button == 4) {
        phaseRender->moveBackward();
    }
}

void keyPressed(unsigned char key, int a, int b)
{
    if (key == 'r')
        generateData();
    else if (key == 'w') {
        phaseRender->moveForward();
    } else if (key == 's') {
        phaseRender->moveBackward();
    } else if (key == 'a') {
        drawnAxis = (Axis)((drawnAxis+1)%AXIS_NELEMS);
        phaseRender->setProjAxes(solver.projectionAxes(drawnAxis));
        glutPostRedisplay();
    } else if (key == 'c') {
        coloredBody = (coloredBody + 1)%3;
        updateColors();
    }
}

// main function
int main(int argc, char **argv)
{
    srand(time(NULL));
    //    srand(time(NULL));
    // initialize glut
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

    // set window size
    glutInitWindowSize(700, 500);
    // set window location
    glutInitWindowPosition(250, 50);

    // create window with window text
    glutCreateWindow("Phase Space Visualizer");
    glewInit();

    initGLRendering();
    generateData();
    glutDisplayFunc(display);
    //  glutIdleFunc(updateRender);
    glutReshapeFunc(reshape);
    glutMotionFunc(mouseDrag);
    glutMouseFunc(mouseClick);
    glutKeyboardFunc(keyPressed);
    glutMainLoop();
    return 0;
}
