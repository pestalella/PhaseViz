#pragma once

#include "ThreeBodySolver.h"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>

class RenderGL {
public:
    RenderGL();
    ~RenderGL();
    void update();
    void display();
    void reshape(int width, int height);
    void mouseDrag(int dx, int dy);
    void moveForward();
    void moveBackward();
    void updateData(std::vector<std::vector<float>> const& lines,
                    std::vector<std::vector<float>> const& colors);
    void setProjAxes(glm::mat3 const &axes);

private:
    unsigned int numPoints;
    unsigned int numLines;

    GLuint vboId;
    GLuint iboId;
    GLuint shaderId;

    glm::vec3 eye;
    glm::mat4 modelMat;
    glm::mat4 modelMatInv;
    glm::mat4 modelViewMatInv;
    glm::mat4 modelViewProjMat;
    glm::mat4 projMat;
    glm::mat4 viewMat;
    
    glm::mat3 projAxes;
//    ThreeBodySolver solv3;
};

