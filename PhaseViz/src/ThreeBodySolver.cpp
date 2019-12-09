#include "ThreeBodySolver.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>

SystemAccels computeAccelerations(ThreeBodySystem s)
{
    glm::dvec3 dir12 = s.body1.position - s.body0.position;
    glm::dvec3 dir21 = s.body0.position - s.body1.position;
    glm::dvec3 dir13 = s.body2.position - s.body0.position;
    glm::dvec3 dir31 = s.body0.position - s.body2.position;
    glm::dvec3 dir23 = s.body2.position - s.body1.position;
    glm::dvec3 dir32 = s.body1.position - s.body2.position;
    SystemAccels result;
    double dist12 = pow(glm::length(dir12), 3.0);
    double dist13 = pow(glm::length(dir13), 3.0);
    double dist23 = pow(glm::length(dir23), 3.0);
    result.a1 = -dir21/dist12 - dir31/dist13;
    result.a2 = -dir32/dist23 - dir12/dist12;
    result.a3 = -dir13/dist13 - dir23/dist23;
    return result;
}

ThreeBodySolver::ThreeBodySolver() :
    occupancy(10*10*10, 0)
{
}

void ThreeBodySolver::updateOccupancy(glm::vec3 const &p)
{

}

bool ThreeBodySolver::isOccupied(glm::vec3 const &p)
{
    return false;
}

std::vector<std::vector<float>> ThreeBodySolver::computeOrbit(
    ThreeBodySystem &tbs, int numPoints,
    glm::vec3 &minCorner, glm::vec3 &maxCorner)
{
    std::vector<float> orbitVertices;
    std::vector<float> orbitColor;

    int numSteps = 0;
    int numVerts = 0;

    glm::vec3 color = randomVector(0.5) + glm::dvec3(0.5);

    glm::vec3 lastVert(0);
    glm::vec3 lastOrbitPoint(0);
    minCorner = glm::vec3(1E10);
    maxCorner = glm::vec3(-1E10);

    double tStep = 0.01;

    auto prevTime = std::chrono::high_resolution_clock::now();
    while (numVerts < numPoints) {
        advanceStep(tbs, tStep);
        glm::vec3 projected = p.phaseSpaceToVizSpace(tbs);
        double distToLastPoint = glm::length(projected - lastOrbitPoint);

        if (distToLastPoint > 1E-3 && numSteps != 0) {
            // Undo last step 
            advanceStep(tbs, -tStep);
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

void ThreeBodySolver::advanceStep(ThreeBodySystem &tbs, double tStep)
{
    auto accels = computeAccelerations(tbs);
    tbs.body0.position += tStep * (tbs.body0.velocity + tStep / 2.0 * accels.a1);
    tbs.body1.position += tStep * (tbs.body1.velocity + tStep / 2.0 * accels.a2);
    tbs.body2.position += tStep * (tbs.body2.velocity + tStep / 2.0 * accels.a3);
    auto newAccels = computeAccelerations(tbs);
    tbs.body0.velocity += tStep / 2.0 * (accels.a1 + newAccels.a1);
    tbs.body1.velocity += tStep / 2.0 * (accels.a2 + newAccels.a2);
    tbs.body2.velocity += tStep / 2.0 * (accels.a3 + newAccels.a3);
}

glm::mat3 ThreeBodySolver::projectionAxes(Axis selectedAxis)
{
    auto pmat = p.projMatrix(selectedAxis);
    auto x = glm::normalize(pmat[0]);
    auto y = glm::normalize(pmat[1]);
    auto z = glm::normalize(pmat[2]);
    return glm::mat3(x, y, z);
}

glm::vec3 ThreeBodySolver::projectSystem(ThreeBodySystem const &tbs)
{
    return p.phaseSpaceToVizSpace(tbs);
}
