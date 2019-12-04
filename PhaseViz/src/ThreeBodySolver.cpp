#include "ThreeBodySolver.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
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
    int numPoints, glm::dvec3 &minCorner, glm::dvec3 &maxCorner)
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

    glm::dvec3 lastVert(0);
    glm::dvec3 lastOrbitPoint(0);

    double tStep = 0.01;


    minCorner = glm::dvec3(1E10);
    maxCorner = glm::dvec3(-1E10);
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
            minCorner.x = (std::min)(minCorner.x, projected.x);
            minCorner.y = (std::min)(minCorner.y, projected.y);
            minCorner.z = (std::min)(minCorner.z, projected.z);
            maxCorner.x = (std::max)(maxCorner.x, projected.x);
            maxCorner.y = (std::max)(maxCorner.y, projected.y);
            maxCorner.z = (std::max)(maxCorner.z, projected.z);
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
        "        Steps/s:" << numSteps*1000.0/duration << std::endl <<
        "        Corners:" << "[" << std::setprecision(3) << minCorner.x << ", " << minCorner.y << ", " << minCorner.z << "]" << std::endl <<
        "                " << "[" << std::setprecision(3) << maxCorner.x << ", " << maxCorner.y << ", " << maxCorner.z << "]" << std::endl;

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
