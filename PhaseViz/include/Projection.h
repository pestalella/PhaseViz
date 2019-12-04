#pragma once

#include <glm/glm.hpp>

class Body;

enum Axis {
    POS0, VEL0, POS1, VEL1, POS2, VEL2, AXIS_NELEMS
};

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

