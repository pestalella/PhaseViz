#include<Windows.h>    
// first include Windows.h header file which is required    


#include "RenderGL.h"

#include <cmath>
#include <stdio.h>    
#include <GL/glew.h>
#include <gl/glut.h>
#include <conio.h>    
#include <stdio.h>    
//#include <string.h>    


#include <iostream>
#include <memory>

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

void generateData()
{
    std::vector<float>::size_type numVerts = 100;
    std::vector<float> positions(numVerts * 3);
    for (int i = 0; i < numVerts / 2; ++i)
    {
        float theta = 2.0 * M_PI * i / (double)numVerts;
        positions[3 * i + 0] = cos(theta);
        positions[3 * i + 1] = sin(theta);
        positions[3 * i + 2] = 1.5 * i / numVerts;
    }
    for (int i = numVerts / 2; i < numVerts; ++i)
    {
        float theta = 2.0 * M_PI * i / (double)numVerts;
        positions[3 * i + 0] = theta;
        positions[3 * i + 1] = theta;
        positions[3 * i + 2] = 1.5 * (i - numVerts/2) / numVerts;
    }
    phaseRender->updateData(positions, 1);
}

// main function    
int main(int argc, char** argv)
{
    // initialize glut    
    glutInit(&argc, argv);

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_MULTISAMPLE);

    // set window size    
    glutInitWindowSize(700, 500);
    // set window location    
    glutInitWindowPosition(250, 50);

    // create window with window text    
    glutCreateWindow("OpenGL Demo");
    glewInit();


    initGLRendering();
    generateData();
    glutDisplayFunc(display);
    // call glutReshapeFunc() function & pass parameter as Reshape() function    
    glutReshapeFunc(reshape);
    glutMotionFunc(mouseDrag);
    //glutMainLoop() is used to redisplay the objects    
    glutMainLoop();
    return 0;
}