#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
const double M_PI = 3.141592653589793238462643;
#elif defined __unix__ 
#include <unistd.h>
#define Sleep(a) usleep((a)*1000)
#endif

#include "RenderGL.h"
#include "ThreeBodySolver.h"

#include <GL/glew.h>
#include <GL/glut.h>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <memory>

std::shared_ptr<RenderGL> phaseRender;
ThreeBodySolver solver;
Axis drawnAxis = Axis::POS0;

void generateData()
{
    std::cout << "Generating data..." << std::endl;
    int numLines = 10;

    std::vector<std::vector<float>> lines;
    std::vector<std::vector<float>> colors;
    glm::dvec3 center(0);
    for (int curLine = 0; curLine < numLines; ++curLine) {
        glm::dvec3 orbitCenter;
        auto orbits = solver.randomSolution(4000, orbitCenter);
        std::cout << "line " << curLine << std::setprecision(3) << " center: ["
            << orbitCenter.x << ", " << orbitCenter.y << ", " << orbitCenter.z
            << "]" << std::endl;
        center += orbitCenter;
        lines.push_back(orbits[0]);
        colors.push_back(orbits[1]);
    }

    center *= 1.0/numLines;

    for (int curLine = 0; curLine < numLines; ++curLine) {
        int numVerts = lines[curLine].size()/3;
        for (int i = 0; i < numVerts; ++i) {
            lines[curLine][3*i + 0] -= center.x;
            lines[curLine][3*i + 1] -= center.y;
            lines[curLine][3*i + 2] -= center.z;
        }
    }

    std::cout << "Sending data to renderer." << std::endl;
    phaseRender->updateData(lines, colors);
    phaseRender->setProjAxes(solver.projectionAxes(drawnAxis));
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
    glClearColor(0.0, 0.0, 1.0, 0.0);
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
    }
}

// main function
int main(int argc, char **argv)
{
    srand(42);
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
