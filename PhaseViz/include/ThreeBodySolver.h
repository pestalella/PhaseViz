#pragma once

#include "Projection.h"

#include <glm/glm.hpp>
#include <vector>

struct Body {
    glm::dvec3 position;
    glm::dvec3 velocity;
};

struct SystemAccels {
    glm::dvec3 a1, a2, a3;
};

class ThreeBodySolver {
public:
    ThreeBodySolver();
    std::vector<std::vector<float>> randomSolution(int numSteps,
        glm::dvec3 &minCorner, glm::dvec3 &maxCorner);
    void advanceStep(double tStep);
    glm::mat3 projectionAxes(Axis selectedAxis);

    Body bodies[3];

private:
    Projection p;
    int occupancy[10][10][10];
};
