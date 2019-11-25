#include "RenderGL.h"

#include <fstream>
#include <GL/glew.h>
#include <gl/glut.h>
#include <glm/ext.hpp>
#include <iostream>
#include <sstream>

//#pragma warning(disable:C26451)

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
    eye(-0.3, 0.5, 5.0),
    modelMat(glm::mat4(1.0f)),
    modelMatInv(glm::mat4(1.0f)),
    modelViewProjMat(glm::mat4(1.0f)),
    projMat(glm::mat4(1.0f)),
    viewMat(glm::mat4(1.0f))
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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shaderId);
    GLuint mvpId = glGetUniformLocation(shaderId, "modelViewProjMatrix");
    glUniformMatrix4fv(mvpId, 1, GL_FALSE, &modelViewProjMat[0][0]);

    // bind VBOs before drawing
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);

    // enable vertex arrays
    glEnableVertexAttribArray(0);          // activate vertex position array
    glEnableVertexAttribArray(1);          // activate vertex color array
    glEnableVertexAttribArray(2);          // activate vertex normal array

    size_t cOffset = sizeof(float) * numPoints * 3;
    size_t nOffset = sizeof(float) * numPoints * 3 + cOffset;

    // specify vertex arrays with their offsets
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, (void*)cOffset);
    glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, (void*)nOffset);

    // finally draw a cube with glDrawElements()
    for (int i = 0; i < numLines; ++i) {
        int indexOffset = 4 * i * numPoints / numLines;
        glDrawElements(GL_LINE_STRIP,
            numPoints/numLines,
            GL_UNSIGNED_INT,         // data type
            (void *)static_cast<intptr_t>(indexOffset));         // offset to indices
    }
    // disable vertex arrays
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    // unbind VBOs
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glutSwapBuffers();
}

void RenderGL::reshape(int width, int height)
{
    std::cout << "Reshape called. width=" << width << " height=" << height << std::endl;
    //adjusts the pixel rectangle for drawing to be the entire new window    
    glViewport(0, 0, (GLsizei)width, (GLsizei)height);

    projMat = glm::perspective((float)(15.0*M_PI/180), (GLfloat)width / (GLfloat)height, 0.1f, 2000.0f);
    viewMat = glm::lookAt(eye, 
                          glm::vec3( 0.0, 0.0, 0.0), 
                          glm::vec3( 0.0, 1.0, 0.0));
    modelViewProjMat = projMat * viewMat * modelMat;
}

void RenderGL::mouseDrag(int dx, int dy)
{
    float rotX = static_cast<float>(dx * M_PI / 180.0);
    float rotY = static_cast<float>(dy * M_PI / 180.0);
    modelMat = glm::rotate(modelMat, rotX, glm::vec3(modelMatInv * glm::vec4(0.0, 1.0, 0.0, 0.0)));
    modelMat = glm::rotate(modelMat, rotY, glm::vec3(modelMatInv * glm::vec4(1.0, 0.0, 0.0, 0.0)));
    modelMatInv = glm::inverse(modelMat);
    modelViewProjMat = projMat * viewMat * modelMat;
    display();
}

void RenderGL::moveForward()
{
    float step = eye.z * 0.1f;
    eye += glm::vec3(0,0,step);
    viewMat = glm::lookAt(eye,
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0));
    modelViewProjMat = projMat * viewMat * modelMat;
    display();
}

void RenderGL::moveBackward()
{
    float step = eye.z * 0.1f;
    eye += glm::vec3(0, 0, -step);
    viewMat = glm::lookAt(eye,
        glm::vec3(0.0, 0.0, 0.0),
        glm::vec3(0.0, 1.0, 0.0));
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

    numPoints = static_cast<int>(lines.size() * lines[0].size() / 3);
    numLines = static_cast<int>(lines.size());

    std::vector<float> vertices;
    std::vector<float> colors;
    std::vector<float> normals;
    std::vector<float> tangents;
    std::vector<unsigned int> indices;
    
    colors.resize(numPoints * 3);
    normals.resize(numPoints * 3);
    tangents.resize(numPoints * 3);
    indices.resize(numPoints);

    for (auto line : lines) {
        vertices.insert(
            vertices.end(),
            std::make_move_iterator(line.begin()),
            std::make_move_iterator(line.end())
        );
    }
    std::cout << "== Stats:" << std::endl;
    std::cout << "==              Num. lines:" << lines.size() << std::endl;
    std::cout << "==    Num. verts per lines:" << lines[0].size() << std::endl;
    std::cout << "==          Total vertices:" << numPoints << std::endl;
    std::cout << "== Computing normals..." << std::endl;
    int curVertexIndex = 0;
    for (int curLine = 0; curLine < numLines; ++curLine) {
        float color[3] = { rand() / (RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f), rand() / (RAND_MAX + 1.0f) };

        int curLineVerts = static_cast<int>(lines[curLine].size() / 3);
        for (int i = 0; i < curLineVerts; ++i)
        {
            indices[curVertexIndex] = curVertexIndex;
            int attribIndex = 3 * curVertexIndex + 0;
            colors[attribIndex + 0] = color[0];
            colors[attribIndex + 1] = color[1];
            colors[attribIndex + 2] = color[2];

            if (i != curLineVerts - 1) {
                // Well, those are tangents
                tangents[attribIndex + 0] = vertices[attribIndex + 3 + 0] - vertices[attribIndex + 0];
                tangents[attribIndex + 1] = vertices[attribIndex + 3 + 1] - vertices[attribIndex + 1];
                tangents[attribIndex + 2] = vertices[attribIndex + 3 + 2] - vertices[attribIndex + 2];
            } else { 
                // The last tangent
                tangents[attribIndex + 0] = 2*tangents[attribIndex - 3 + 0] - tangents[attribIndex - 6 + 0];
                tangents[attribIndex + 1] = 2*tangents[attribIndex - 3 + 1] - tangents[attribIndex - 6 + 1];
                tangents[attribIndex + 2] = 2*tangents[attribIndex - 3 + 2] - tangents[attribIndex - 6 + 2];

            }
            curVertexIndex++;
        }

        for (int i = 0; i < numPoints-1; ++i) {
            glm::vec3 t1(tangents[3 * i + 0], tangents[3 * i + 1], tangents[3 * i + 2]);
            glm::vec3 t2(tangents[3 * (i+1) + 0], tangents[3 * (i+1) + 1], tangents[3 * (i+1) + 2]);
            glm::vec3 up = glm::normalize(glm::cross(t1, t2));
            glm::vec3 normal = glm::normalize(glm::cross(t1, up));
            normals[3 * i + 0] = normal[0];
            normals[3 * i + 1] = normal[1];
            normals[3 * i + 2] = normal[2];
        }
        normals[3*(numPoints - 1) + 0] = normals[3*(numPoints - 2) + 0];
        normals[3*(numPoints - 1) + 1] = normals[3*(numPoints - 2) + 1];
        normals[3*(numPoints - 1) + 2] = normals[3*(numPoints - 2) + 2];
    }
 
    std::cout << "== Normals computed. Sending data to the GPU." << std::endl;

    // copy vertex attribs data to VBO
    glBindBuffer(GL_ARRAY_BUFFER, vboId);
    // reserve space
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * (vertices.size() + colors.size() + normals.size()), 0, GL_STATIC_DRAW);
    // copy positions
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)*vertices.size(), &vertices[0]);
    // copy colors
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size(), sizeof(float)*colors.size(), &colors[0]);
    // copy normals
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(float)*vertices.size() + sizeof(float) * colors.size(), sizeof(float)*normals.size(), &normals[0]);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), &indices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

