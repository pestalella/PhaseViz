#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class RenderGL {
public:
    RenderGL();
    ~RenderGL();
    void display();
    void reshape(int width, int height);
    void mouseDrag(int dx, int dy);
    void updateData(std::vector<float> &data, int nLines);

private:
    int numPoints;
    int numLines;

    GLuint vboId;
    GLuint iboId;
    GLuint shaderId;

    glm::mat4 projMat;
    glm::mat4 modelMat;
    glm::mat4 modelMatInv;
    glm::mat4 viewMat;
    glm::mat4 modelViewProjMat;
};

