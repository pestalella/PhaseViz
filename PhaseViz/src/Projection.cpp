#include "Projection.h"
#include "ThreeBodySolver.h"

#include <numeric>
#include <vector>

Projection::Projection()
{
    createMatrix();
}

void Projection::createMatrix()
{
    std::vector<double> rowX(18);
    std::vector<double> rowY(18);
    std::vector<double> rowZ(18);

    for (int i = 0; i < 18; ++i) {
        rowX[i] = (rand() / (RAND_MAX + 1.0) - 0.5)*2;
        rowY[i] = (rand() / (RAND_MAX + 1.0) - 0.5)*2;
        rowZ[i] = (rand() / (RAND_MAX + 1.0) - 0.5)*2;
    }

    double sumSquaredX = std::inner_product(rowX.begin(), rowX.end(), rowX.begin(), 0.0);
    double sumSquaredY = std::inner_product(rowY.begin(), rowY.end(), rowY.begin(), 0.0);
    double sumSquaredZ = std::inner_product(rowZ.begin(), rowZ.end(), rowZ.begin(), 0.0);
    double sqX = 1.0 / sqrt(sumSquaredX);
    double sqY = 1.0 / sqrt(sumSquaredY);
    double sqZ = 1.0 / sqrt(sumSquaredZ);

    for (int i = 0; i < 18; ++i) {
        rowX[i] *= sqX;
        rowY[i] *= sqY;
        rowZ[i] *= sqZ;
    }

    for (int body = 0; body < 3; body++) {
        for (int column = 0; column < 3; column++) {
            positions[body][column][0] = rowX[3*body + column];
            positions[body][column][1] = rowY[3*body + column];
            positions[body][column][2] = rowZ[3*body + column];
        }
    }
    for (int body = 0; body < 3; body++) {
        for (int column = 0; column < 3; column++) {
            velocities[body][column][0] = rowX[9 + 3*body + column];
            velocities[body][column][1] = rowY[9 + 3*body + column];
            velocities[body][column][2] = rowZ[9 + 3*body + column];
        }
    }
}

glm::dvec3 Projection::phaseSpaceToVizSpace(Body const &b0, Body const &b1, Body const &b2)
{
    glm::dvec3 r = positions[0]*b0.position + positions[1]*b1.position + positions[2]*b2.position +
        velocities[0]*b0.velocity + velocities[1]*b1.velocity + velocities[2]*b2.velocity;
    return r;
}

glm::mat3x3 Projection::projMatrix(Axis selectedAxis)
{
    switch (selectedAxis) {
    case POS0: return positions[0];
    case POS1: return positions[1];
    case POS2: return positions[2];
    case VEL0: return velocities[0];
    case VEL1: return velocities[1];
    case VEL2: return velocities[2];
    }
}

