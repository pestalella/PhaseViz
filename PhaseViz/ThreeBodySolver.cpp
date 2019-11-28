#include "ThreeBodySolver.h"

#include <iomanip>
#include <iostream>
#include <numeric>

Projection::Projection() : rowX(18), rowY(18), rowZ(18) { createMatrix(); }

void Projection::createMatrix()
{
    for (int i = 0; i < 18; ++i) {
        rowX[i] = rand() / (RAND_MAX + 1.0);
        rowY[i] = rand() / (RAND_MAX + 1.0);
        rowZ[i] = rand() / (RAND_MAX + 1.0);
    }
    double sX = 1.0 / std::accumulate(rowX.begin(), rowX.end(), 0.0);
    double sY = 1.0 / std::accumulate(rowY.begin(), rowY.end(), 0.0);
    double sZ = 1.0 / std::accumulate(rowZ.begin(), rowZ.end(), 0.0);

    for (int i = 0; i < 18; ++i) {
        rowX[i] *= sX;
        rowY[i] *= sY;
        rowZ[i] *= sZ;
    }
    std::cout << std::setprecision(3) << "rowX rowY rowZ" << std::endl;
    for (int i = 0; i < 18; ++i) {
        std::cout << std::setprecision(3) << rowX[i] << " " << rowY[i] << " "
            << rowZ[i] << std::endl;
    }
}

glm::vec3 Projection::phaseSpaceToVizSpace(Body const &b1, Body const &b2,
    Body const &b3)
{
    std::vector<float> phaseSpacePoint(18);  // 18 dims

    phaseSpacePoint[0] = b1.position[0];
    phaseSpacePoint[1] = b1.position[1];
    phaseSpacePoint[2] = b1.position[2];
    phaseSpacePoint[3] = b1.velocity[0];
    phaseSpacePoint[4] = b1.velocity[1];
    phaseSpacePoint[5] = b1.velocity[2];

    phaseSpacePoint[6] = b2.position[0];
    phaseSpacePoint[7] = b2.position[1];
    phaseSpacePoint[8] = b2.position[2];
    phaseSpacePoint[9] = b2.velocity[0];
    phaseSpacePoint[10] = b2.velocity[1];
    phaseSpacePoint[11] = b2.velocity[2];

    phaseSpacePoint[12] = b3.position[0];
    phaseSpacePoint[13] = b3.position[1];
    phaseSpacePoint[14] = b3.position[2];
    phaseSpacePoint[15] = b3.velocity[0];
    phaseSpacePoint[16] = b3.velocity[1];
    phaseSpacePoint[17] = b3.velocity[2];

    float x = std::inner_product(std::begin(phaseSpacePoint),
        std::end(phaseSpacePoint),
        std::begin(rowX), 0.0);
    float y = std::inner_product(std::begin(phaseSpacePoint),
        std::end(phaseSpacePoint),
        std::begin(rowY), 0.0);
    float z = std::inner_product(std::begin(phaseSpacePoint),
        std::end(phaseSpacePoint),
        std::begin(rowZ), 0.0);

    return glm::vec3(x, y, z);
}
glm::dvec3 randomVector(double scale = 1.0)
{
    return glm::dvec3((rand() / (RAND_MAX + 1.0) - 0.5) * scale,
        (rand() / (RAND_MAX + 1.0) - 0.5) * scale,
        (rand() / (RAND_MAX + 1.0) - 0.5) * scale);
}

Body randomBody()
{
    return {randomVector(0.1), randomVector(0)};
}

SystemAccels computeAccelerations(Body const &b1, Body const &b2,
    Body const &b3)
{
    glm::dvec3 dir12 = b2.position - b1.position;
    glm::dvec3 dir21 = b1.position - b2.position;
    glm::dvec3 dir13 = b3.position - b1.position;
    glm::dvec3 dir31 = b1.position - b3.position;
    glm::dvec3 dir23 = b3.position - b2.position;
    glm::dvec3 dir32 = b2.position - b3.position;
    SystemAccels result;
    double dist12 = pow(glm::length(dir12), 3.0);
    double dist13 = pow(glm::length(dir13), 3.0);
    double dist23 = pow(glm::length(dir23), 3.0);
    result.a1 = -dir21/dist12 - dir31/dist13;
    result.a2 = -dir32/dist23 - dir12/dist12;
    result.a3 = -dir13/dist13 - dir23/dist23;
    return result;
}

ThreeBodySolver::ThreeBodySolver()
{
    bodies[0] = randomBody();
    bodies[1] = randomBody();
    bodies[2] = randomBody();
}

std::vector<std::vector<float>> ThreeBodySolver::randomSolution(int numPoints)
{
    std::vector<float> orbitVertices;
    std::vector<float> orbitColor;

    bodies[0] = randomBody();
    bodies[1] = randomBody();
    bodies[2] = randomBody();

    const glm::vec3 bias1(10, -10, 0);
    const glm::vec3 bias2(-10, 0, 0);
    const glm::vec3 bias3(10, 10, 0);
    bodies[0].position += bias1;
    bodies[1].position += bias2;
    bodies[2].position += bias3;

    glm::dvec3 center = 1/3.0*(bodies[0].position + bodies[1].position + bodies[2].position);
    //  glm::dvec3 center(0);
    glm::vec3 velocity = 1/3.0*(bodies[0].velocity + bodies[1].velocity + bodies[2].velocity);
//    glm::dvec3 velocity(0);

      // Center the system around the origin of coords
    bodies[0].position -= center;
    bodies[1].position -= center;
    bodies[2].position -= center;
    // Remove system velocity
    bodies[0].velocity -= velocity;
    bodies[1].velocity -= velocity;
    bodies[2].velocity -= velocity;

    int numSteps = 0;
    int numVerts = 0;

    glm::vec3 color = randomVector(0.5) + glm::dvec3(0.5);

    // for (int i = 0; i < numSteps; ++i) {
    while (numVerts < numPoints) {
        advanceStep(0.01);

        numSteps++;
//        if (numSteps % 10 == 1) {
            auto projected = p.phaseSpaceToVizSpace(bodies[0], bodies[1], bodies[2]);
            orbitVertices.push_back(projected[0]);
            orbitVertices.push_back(projected[1]);
            orbitVertices.push_back(projected[2]);

            orbitColor.push_back(color.r);
            orbitColor.push_back(color.g);
            orbitColor.push_back(color.b);

            numVerts++;
//        }
    }

    return std::vector<std::vector<float>>({orbitVertices, orbitColor});
}

void ThreeBodySolver::advanceStep(double tStep)
{
    auto accels = computeAccelerations(bodies[0], bodies[1], bodies[2]);
    bodies[0].position += tStep * (bodies[0].velocity + tStep / 2.0 * accels.a1);
    bodies[1].position += tStep * (bodies[1].velocity + tStep / 2.0 * accels.a2);
    bodies[2].position += tStep * (bodies[2].velocity + tStep / 2.0 * accels.a3);
    auto newAccels = computeAccelerations(bodies[0], bodies[1], bodies[2]);
    bodies[0].velocity += tStep / 2.0 * (accels.a1 + newAccels.a1);
    bodies[1].velocity += tStep / 2.0 * (accels.a2 + newAccels.a2);
    bodies[2].velocity += tStep / 2.0 * (accels.a3 + newAccels.a3);
}
