#include<Windows.h>    
// first include Windows.h header file which is required    

#include "RenderGL.h"
#include "ThreeBodySolver.h"

#include <cmath>
#include <GL/glew.h>
#include <gl/glut.h>
#include <iostream>
#include <memory>
#include <ctime>

const double M_PI = 3.141592653589793238462643;

std::shared_ptr<RenderGL> phaseRender;

// Display_Objects() function    
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
    glClearColor(0.0, 0.0, 0.0, 0.0);
    // set shade model to Flat    
    glShadeModel(GL_FLAT);
    phaseRender = std::make_shared<RenderGL>();
}

void mouseDrag(int x, int y)
{
    static int prevX = 0, prevY = 0;
    int offsetX = x - prevX;
    int offsetY = y - prevY;
    prevX = x;
    prevY = y;
    phaseRender->mouseDrag(offsetX, offsetY);
}

void mouseClick(int button, int state, int x, int y)
{
    if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
    // Wheel reports as button 3(scroll up) and button 4(scroll down)
    if (button == 3) { // It's a wheel event
        phaseRender->moveBackward();
    } else if (button == 4) {
        phaseRender->moveForward();
    }
}


void generateData()
{
    std::cout << "Generating data..." << std::endl;
    int numLines = 100;
    ThreeBodySolver solver;
    
    std::vector<std::vector<float>> lines;
    for (int curLine = 0; curLine < numLines; ++curLine) {
        auto orbits = solver.randomSolution(2000);
        for (auto line : orbits) {
            lines.push_back(line);
        }
    }


    //std::vector<float>::size_type numVerts = 2000;
    //std::vector<std::vector<float>> lines;

    //for (int curLine = 0; curLine < numLines; ++curLine) {
    //    std::vector<float> positions(numVerts*3);
    //    int r = rand() / (RAND_MAX + 1.0) * 2.0;
    //    for (int i = 0; i < numVerts; ++i) {
    //        float theta = 6*2.0*M_PI*i/(double)numVerts;
    //        positions[3 * i + 0] = r*cos(theta*(curLine+1)/20);
    //        positions[3 * i + 1] = r*sin(theta*(curLine + 1) / r);
    //        positions[3 * i + 2] = 3.0*i/numVerts + 0.001*curLine;
    //    }
    //    lines.push_back(positions);
    //}
    std::cout << "Sending data to renderer." << std::endl;
    phaseRender->updateData(lines);
}



// main function    
int main(int argc, char** argv)
{
    srand(time(NULL));
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
    // call glutReshapeFunc() function & pass parameter as Reshape() function    
    glutReshapeFunc(reshape);
    glutMotionFunc(mouseDrag);
    glutMouseFunc(mouseClick);
    glutMainLoop();
    return 0;
}