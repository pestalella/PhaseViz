#include "RenderGL.h"

#include <fstream>
#include <GL/glew.h>
#include <gl/glut.h>
#include <glm/ext.hpp>
#include <iostream>
#include <sstream>

const double M_PI = 3.141592653589793238462643;

GLuint loadShaders(const char* vertex_file_path, const char* fragment_file_path) {

    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader : %s\n", vertex_file_path);
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path);
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

RenderGL::RenderGL() :
    numPoints(0),
    numLines(0),
    vboId(0),
    iboId(0),
    shaderId(0),
    modelMat(glm::mat4(1.0f)),
    modelMatInv(glm::mat4(1.0f)),
    viewMat(glm::mat4(1.0f)),
    projMat(glm::mat4(1.0f)),
    modelViewProjMat(glm::mat4(1.0f))
{
    shaderId = loadShaders("rotate.vert", "phase.frag");
}

RenderGL::~RenderGL()
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);
    glDeleteProgram(shaderId);
}

void RenderGL::display()
{
    // clearing the window or remove all drawn objects    
    glClear(GL_COLOR_BUFFER_BIT);
    // bind VBOs before drawing
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    GLuint mvpId = glGetUniformLocation(shaderId, "modelViewProjMatrix");
    glUniformMatrix4fv(mvpId, 1, GL_FALSE, &modelViewProjMat[0][0]);

    glUseProgram(shaderId);
    // enable vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);

    size_t cOffset = sizeof(float)* numPoints*3;

    // specify vertex arrays with their offsets
    glVertexPointer(3, GL_FLOAT, 0, (void*)0);
    glColorPointer(3, GL_FLOAT, 0, (void*)cOffset);

    // finally draw a cube with glDrawElements()
    for (int i = 0; i < numLines; ++i) {
        int indexOffset = 4 * i * numPoints / numLines;
        glDrawElements(GL_LINE_STRIP,
            numPoints/numLines,
            GL_UNSIGNED_INT,         // data type
            (void *)indexOffset);         // offset to indices
    }
    // disable vertex arrays
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    // unbind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glutSwapBuffers();
}

void RenderGL::reshape(int width, int height)
{
    //adjusts the pixel rectangle for drawing to be the entire new window    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    projMat = glm::perspective((float)(60.0*M_PI/180), (GLfloat)width / (GLfloat)height, 0.5f, 20.0f);
    viewMat = glm::lookAt(glm::vec3(-0.3, 0.5, 5.0), 
                          glm::vec3( 0.0, 0.0, 0.0), 
                          glm::vec3( 0.0, 1.0, 0.0));
    modelViewProjMat = projMat * viewMat * modelMat;
}

void RenderGL::mouseDrag(int dx, int dy)
{
    float rotX = dx * M_PI / 180.0;
    float rotY = dy * M_PI / 180.0;
    modelMat = glm::rotate(modelMat, rotX, glm::vec3(modelMatInv * glm::vec4(0.0, 1.0, 0.0, 0.0)));
    modelMat = glm::rotate(modelMat, rotY, glm::vec3(modelMatInv * glm::vec4(1.0, 0.0, 0.0, 0.0)));
    modelMatInv = glm::inverse(modelMat);
    modelViewProjMat = projMat * viewMat * modelMat;
    display();
}

void RenderGL::updateData(std::vector<float>& vertices, int nLines)
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);

    // create VBOs
    glGenBuffersARB(1, &vboId);    // for vertex buffer
    glGenBuffers(1, &iboId);    // for index buffer

    std::vector<float> colors;
    std::vector<unsigned int> indices;
    colors.resize(vertices.size());
    numPoints = vertices.size() / 3;
    numLines = nLines;
    indices.resize(numPoints);
    for (int i = 0; i < numPoints; ++i)
    {
        indices[i] = i;
        float theta = 2.0 * M_PI * i / (double)numPoints;
        if (i < numPoints / 2) {
            colors[3 * i + 0] = 0.0;
            colors[3 * i + 1] = 1.0;
            colors[3 * i + 2] = 0.0;
        }
        else {
            colors[3 * i + 0] = 1.0;
            colors[3 * i + 1] = 0.0;
            colors[3 * i + 2] = 1.0;

        }
    }
 
    // copy vertex attribs data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    // reserve space
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (vertices.size() + colors.size()), 0, GL_STATIC_DRAW);
    // copy positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * vertices.size(), &vertices[0]);
    // copy colors
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), sizeof(float) * colors.size(), &colors[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

