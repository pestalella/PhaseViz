#pragma once


#include <glm/glm.hpp>
#include <vector>


struct Body
{
    glm::dvec3 position;
    glm::dvec3 velocity;
};

struct SystemAccels
{
    glm::dvec3 a1, a2, a3;
};


class Projection
{
public:
    Projection();

    void createMatrix();

    glm::dvec3 phaseSpaceToVizSpace(Body const& b1, Body const& b2, Body const& b3);

private:
    std::vector<double> rowX;
    std::vector<double> rowY;
    std::vector<double> rowZ;
};

class ThreeBodySolver
{
public:
    ThreeBodySolver();
    std::vector<std::vector<float>> randomSolution(int numSteps);

private:
    Projection p;

};

