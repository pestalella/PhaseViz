#if defined(_WIN32) || defined(WIN32)
#include <Windows.h>
#elif defined __unix__ 
#include <unistd.h>
#endif

#include "RenderGL.h"

#include <ctime>
#include <GL/glut.h>

// main function
int main(int argc, char **argv)
{
    srand(time(NULL));
    //    srand(time(NULL));
    // initialize glut
    initGLRendering(argc, argv);
    glutMainLoop();
    return 0;
}
