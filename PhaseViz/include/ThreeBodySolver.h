#pragma once

#include "Projection.h"

#include <glm/glm.hpp>
#include <vector>

struct Body
{
    glm::dvec3 position;
    glm::dvec3 velocity;
};

class ThreeBodySystem
{
public:
    Body body[3];
};

struct SystemAccels
{
    glm::dvec3 a1, a2, a3;
};

class ThreeBodySolver
{
public:
    ThreeBodySolver();
    void updateOccupancy(glm::vec3 const &p);
    bool isOccupied(glm::vec3 const &p);
    std::pair<std::vector<ThreeBodySystem>, std::vector<float>> computeOrbit(ThreeBodySystem &tbs, int numSteps);
    void advanceStep(ThreeBodySystem &tbs, double tStep);
    glm::mat3 projectionAxes(Axis selectedAxis);
    glm::vec3 projectSystem(ThreeBodySystem const &tbs);
    void nextColoredBody() { coloredBody = (coloredBody+1)%3; }

private:
    Projection p;
    std::vector<int> occupancy;
    int coloredBody;
};
