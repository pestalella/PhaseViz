#include "ThreeBodySolver.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>

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
        //rowX[i] = (rand() / (RAND_MAX + 1.0));
        //rowY[i] = (rand() / (RAND_MAX + 1.0));
        //rowZ[i] = (rand() / (RAND_MAX + 1.0));
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

glm::dvec3 randomVector(double scale = 1.0)
{
    return glm::dvec3((rand() / (RAND_MAX + 1.0) - 0.5) * scale,
        (rand() / (RAND_MAX + 1.0) - 0.5) * scale,
        (rand() / (RAND_MAX + 1.0) - 0.5) * scale);
}

Body randomBody()
{
    return { randomVector(0.1), randomVector(0) };
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

std::vector<std::vector<float>> ThreeBodySolver::randomSolution(
    int numPoints, glm::dvec3 &orbitCenter)
{
    std::vector<float> orbitVertices;
    std::vector<float> orbitColor;

    bodies[0] = randomBody();
    bodies[1] = randomBody();
    bodies[2] = randomBody();

    const glm::vec3 bias1(20, 10, 0);
    const glm::vec3 bias2(-10, 0, 0);
    const glm::vec3 bias3(0, -10, 0);
    bodies[0].position += bias1;
    bodies[1].position += bias2;
    bodies[2].position += bias3;

    glm::dvec3 center =
        1/3.0*(bodies[0].position + bodies[1].position + bodies[2].position);
    //  glm::dvec3 center(0);
    glm::vec3 velocity =
        1/3.0*(bodies[0].velocity + bodies[1].velocity + bodies[2].velocity);
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

    glm::dvec3 accum(0);

    orbitCenter = glm::dvec3(0);
    glm::dvec3 lastVert(0);
    glm::dvec3 lastOrbitPoint(0);

    double tStep = 0.01;


    auto prevTime = std::chrono::high_resolution_clock::now();
    while (numVerts < numPoints) {
        advanceStep(tStep);
        auto projected = p.phaseSpaceToVizSpace(bodies[0], bodies[1], bodies[2]);
        double distToLastPoint = glm::length(projected - lastOrbitPoint);

        if (distToLastPoint > 1E-3 && numSteps != 0) {
            // Undo last step 
            advanceStep(-tStep);
            tStep *= 0.5;
            continue;
        } else if (distToLastPoint < 1E-4) {
            // We are doing tiny steps. Make them longer in next iter.
            tStep *= 2;
        }
        double distToLastVert = glm::length(projected - lastVert);
        lastOrbitPoint = projected;

        if ((distToLastVert > 5E-2) ||
            ((distToLastVert > 1E-2) && (numSteps % 100 == 1))) {
            lastVert = projected;
            orbitCenter += projected;
            orbitVertices.push_back(projected[0]);
            orbitVertices.push_back(projected[1]);
            orbitVertices.push_back(projected[2]);

            orbitColor.push_back(color.r);
            orbitColor.push_back(color.g);
            orbitColor.push_back(color.b);

            numVerts++;
        }
        numSteps++;
    }
    auto curTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - prevTime).count();
    std::cout << "* Orbit stats" << std::endl <<
        "---------------" << std::endl <<
        "    Total steps:" << numSteps << std::endl <<
        "       Vertices:" << numVerts << std::endl <<
        "       Time(ms):" << duration << std::endl <<
        "        Steps/s:" << numSteps*1000.0/duration << std::endl;

    orbitCenter *= 1.0/numVerts;
    return std::vector<std::vector<float>>({ orbitVertices, orbitColor });
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

glm::mat3 ThreeBodySolver::projectionAxes(Axis selectedAxis)
{
    auto pmat = p.projMatrix(selectedAxis);
    auto x = glm::normalize(pmat[0]);
    auto y = glm::normalize(pmat[1]);
    auto z = glm::normalize(pmat[2]);
    return glm::mat3(x, y, z);
}
