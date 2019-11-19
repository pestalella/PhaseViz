#include "RenderGL.h"

#include <fstream>
#include <GL/glew.h>
#include <gl/glut.h>
#include <glm/ext.hpp>
#include <iostream>
#include <sstream>

const double M_PI = 3.141592653589793238462643;

GLuint loadShaders(std::string const &vertexFilePath, std::string const &fragmentFilePath)
{
    // Create the shaders
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string vertexShaderCode;
    std::ifstream vertexShaderStream(vertexFilePath, std::ios::in);
    if (vertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << vertexShaderStream.rdbuf();
        vertexShaderCode = sstr.str();
        vertexShaderStream.close();
    } else {
        std::cout << "Unable to open " << vertexFilePath << std::endl;
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string fragmentShaderCode;
    std::ifstream fragmentShaderStream(fragmentFilePath, std::ios::in);
    if (fragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << fragmentShaderStream.rdbuf();
        fragmentShaderCode = sstr.str();
        fragmentShaderStream.close();
    }

    GLint result = GL_FALSE;
    int infoLogLength;

    // Compile Vertex Shader
    std::cout << "Compiling shader : " << vertexFilePath << std::endl;
    char const* vertexSourcePointer = vertexShaderCode.c_str();
    glShaderSource(vertexShaderID, 1, &vertexSourcePointer, NULL);
    glCompileShader(vertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> vertexShaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(vertexShaderID, infoLogLength, NULL, &vertexShaderErrorMessage[0]);
        std::cout << &vertexShaderErrorMessage[0] << std::endl;

    }

    // Compile Fragment Shader
    std::cout << "Compiling shader : " << fragmentFilePath << std::endl;
    char const* fragmentSourcePointer = fragmentShaderCode.c_str();
    glShaderSource(fragmentShaderID, 1, &fragmentSourcePointer, NULL);
    glCompileShader(fragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &result);
    glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> fragmentShaderErrorMessage(infoLogLength + 1);
        glGetShaderInfoLog(fragmentShaderID, infoLogLength, NULL, &fragmentShaderErrorMessage[0]);
        std::cout << &fragmentShaderErrorMessage[0] << std::endl;
    }

    // Link the program
    std::cout << "Linking program" << std::endl;
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    // Check the program
    glGetProgramiv(programID, GL_LINK_STATUS, &result);
    glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &infoLogLength);
    if (infoLogLength > 0) {
        std::vector<char> programErrorMessage(infoLogLength + 1);
        glGetProgramInfoLog(programID, infoLogLength, NULL, &programErrorMessage[0]);
        std::cout << &programErrorMessage[0] << std::endl;
    }

    glDetachShader(programID, vertexShaderID);
    glDetachShader(programID, fragmentShaderID);

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);

    return programID;
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

    glUseProgram(shaderId);
    GLuint mvpId = glGetUniformLocation(shaderId, "modelViewProjMatrix");
    glUniformMatrix4fv(mvpId, 1, GL_FALSE, &modelViewProjMat[0][0]);

    // bind VBOs before drawing
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    // enable vertex arrays
    glEnableVertexAttribArray(0);          // activate vertex position array
    glEnableVertexAttribArray(1);          // activate vertex normal array

    size_t cOffset = sizeof(float)* numPoints*3;

    // specify vertex arrays with their offsets
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void *)cOffset);

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

void RenderGL::updateData(std::vector<std::vector<float>> const &lines)
{
    glDeleteBuffers(1, &vboId);
    glDeleteBuffers(1, &iboId);

    // create VBOs
    glGenBuffersARB(1, &vboId);    // for vertex buffer
    glGenBuffers(1, &iboId);    // for index buffer

    numPoints = lines.size() * lines[0].size() / 3;
    numLines = lines.size();

    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<unsigned int> indices;
    
    colors.resize(numPoints * 3);
    indices.resize(numPoints);

    for (auto line : lines) {
        vertices.insert(
            vertices.end(),
            std::make_move_iterator(line.begin()),
            std::make_move_iterator(line.end())
        );
    }

    int curVertexIndex = 0;
    for (int curLine = 0; curLine < numLines; ++curLine) {
        float color[3] = { rand() / (RAND_MAX + 1.0), rand() / (RAND_MAX + 1.0), rand() / (RAND_MAX + 1.0) };

        int curLineVerts = lines[curLine].size() / 3;
        for (int i = 0; i < curLineVerts; ++i)
        {
            indices[curVertexIndex] = curVertexIndex;
            colors[3 * curVertexIndex + 0] = color[0];
            colors[3 * curVertexIndex + 1] = color[1];
            colors[3 * curVertexIndex + 2] = color[2];
            curVertexIndex++;
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

