#pragma once

#include "Projection.h"

#include <glm/glm.hpp>
#include <vector>

struct Body 
{
    glm::dvec3 position;
    glm::dvec3 velocity;
};

struct ThreeBodySystem
{
    Body body0, body1, body2;
};

struct SystemAccels 
{
    glm::dvec3 a1, a2, a3;
};

class ThreeBodySolver 
{
public:
    ThreeBodySolver();
    void updateOccupancy(glm::dvec3 const &p);
    bool isOccupied(glm::dvec3 const &p);
    std::vector<std::vector<float>> randomSolution(int numSteps,
        glm::dvec3 &minCorner, glm::dvec3 &maxCorner);
    void advanceStep(double tStep);
    glm::mat3 projectionAxes(Axis selectedAxis);

    ThreeBodySystem tbs;

private:
    Projection p;
    std::vector<int> occupancy;
};
