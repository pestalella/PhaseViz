#include "ThreeBodySolver.h"
#include "utils.h"

#include <algorithm>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <utility>

SystemAccels computeAccelerations(ThreeBodySystem s)
{
    glm::dvec3 dir01 = s.body[1].position - s.body[0].position;
    glm::dvec3 dir10 = s.body[0].position - s.body[1].position;
    glm::dvec3 dir02 = s.body[2].position - s.body[0].position;
    glm::dvec3 dir20 = s.body[0].position - s.body[2].position;
    glm::dvec3 dir12 = s.body[2].position - s.body[1].position;
    glm::dvec3 dir21 = s.body[1].position - s.body[2].position;
    SystemAccels result;
    double dist12 = pow(glm::length(dir01), 3.0);
    double dist13 = pow(glm::length(dir02), 3.0);
    double dist23 = pow(glm::length(dir12), 3.0);
    result.a1 = -dir10/dist12 - dir20/dist13;
    result.a2 = -dir21/dist23 - dir01/dist12;
    result.a3 = -dir02/dist13 - dir12/dist23;
    return result;
}

ThreeBodySolver::ThreeBodySolver() :
    occupancy(10*10*10, 0)
//    coloredBody(0)
{
}

void ThreeBodySolver::updateOccupancy(glm::vec3 const &p)
{

}

bool ThreeBodySolver::isOccupied(glm::vec3 const &p)
{
    return false;
}

std::pair<std::vector<ThreeBodySystem>, std::vector<float>> ThreeBodySolver::computeOrbit(ThreeBodySystem &tbs, int numPoints)
{
    std::vector<ThreeBodySystem> orbitStates;
    std::vector<float> orbitVertices;
    std::vector<float> orbitColor;

    int numSteps = 0;
    int numVerts = 0;

    glm::vec3 color = randomVector(0.5) + glm::dvec3(0.5);

    glm::vec3 lastVert(0);
    glm::vec3 lastOrbitPoint(0);

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
        if ((distToLastVert > 1E-1) ||
            ((distToLastVert > 5E-2) && (numSteps % 100 == 1))) {
            lastVert = projected;
            orbitVertices.push_back(projected[0]);
            orbitVertices.push_back(projected[1]);
            orbitVertices.push_back(projected[2]);
            orbitStates.push_back(tbs);
            numVerts++;
        }
        numSteps++;
    }
    auto curTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(curTime - prevTime).count();
    // std::cout << "* Orbit stats" << std::endl <<
    //     "---------------" << std::endl <<
    //     "    Total steps:" << numSteps << std::endl <<
    //     "       Vertices:" << numVerts << std::endl <<
    //     "       Time(ms):" << duration << std::endl <<
    //     "        Steps/s:" << numSteps*1000.0/duration << std::endl;

    return std::make_pair(orbitStates, orbitVertices);
}

void ThreeBodySolver::advanceStep(ThreeBodySystem &tbs, double tStep)
{
    auto accels = computeAccelerations(tbs);
    tbs.body[0].position += tStep * (tbs.body[0].velocity + tStep / 2.0 * accels.a1);
    tbs.body[1].position += tStep * (tbs.body[1].velocity + tStep / 2.0 * accels.a2);
    tbs.body[2].position += tStep * (tbs.body[2].velocity + tStep / 2.0 * accels.a3);
    auto newAccels = computeAccelerations(tbs);
    tbs.body[0].velocity += tStep / 2.0 * (accels.a1 + newAccels.a1);
    tbs.body[1].velocity += tStep / 2.0 * (accels.a2 + newAccels.a2);
    tbs.body[2].velocity += tStep / 2.0 * (accels.a3 + newAccels.a3);
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
