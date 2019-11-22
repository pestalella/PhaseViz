#pragma once


#include <glm/glm.hpp>
#include <vector>


struct Body
{
    glm::vec3 position;
    glm::vec3 velocity;
};

struct SystemAccels
{
    glm::vec3 a1, a2, a3;
};


class Projection
{
public:
    Projection();

    void createMatrix();

    glm::vec3 phaseSpaceToVizSpace(Body const& b1, Body const& b2, Body const& b3);

private:
    std::vector<float> rowX;
    std::vector<float> rowY;
    std::vector<float> rowZ;
};

class ThreeBodySolver
{
public:
    ThreeBodySolver();
    std::vector<std::vector<float>> randomSolution(int numSteps);

private:
    Projection p;

};

