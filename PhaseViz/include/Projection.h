#pragma once

#include <glm/glm.hpp>

class ThreeBodySystem;

enum Axis {
    POS0, VEL0, POS1, VEL1, POS2, VEL2, AXIS_NELEMS
};

class Projection {
public:
    Projection();

    void createMatrix();

    glm::dvec3 phaseSpaceToVizSpace(ThreeBodySystem const &s);
    glm::mat3x3 projMatrix(Axis selectedAxis);
private:
    glm::mat3x3 positions[3];
    glm::mat3x3 velocities[3];
};

