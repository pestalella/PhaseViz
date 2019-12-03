#pragma once

#include <glm/glm.hpp>
#include <vector>

struct Body {
    glm::dvec3 position;
    glm::dvec3 velocity;
};

struct SystemAccels {
    glm::dvec3 a1, a2, a3;
};

enum Axis { POS0, VEL0, POS1, VEL1, POS2, VEL2, AXIS_NELEMS };

class Projection {
public:
    Projection();

    void createMatrix();

    glm::dvec3 phaseSpaceToVizSpace(Body const &b0, Body const &b1,
        Body const &b2);
    glm::mat3x3 projMatrix(Axis selectedAxis);
private:
    glm::mat3x3 positions[3];
    glm::mat3x3 velocities[3];
};

class ThreeBodySolver {
public:
    ThreeBodySolver();
    std::vector<std::vector<float>> randomSolution(int numSteps,
        glm::dvec3 &orbitCenter);
    void advanceStep(double tStep);
    glm::mat3 projectionAxes(Axis selectedAxis);

    Body bodies[3];

private:
    Projection p;
};
