#include "OrbitGenerator.h"
#include "ThreeBodySolver.h"
#include "utils.h"

#include <iostream>

ThreeBodySolver solver;
std::vector<std::vector<ThreeBodySystem>> states;

ThreeBodySystem randomSystem()
{
    float radius = 0.1;
    Body body0 = {randomVector(radius), randomVector(0)};
    Body body1 = {randomVector(radius), randomVector(0)};
    Body body2 = {randomVector(radius), randomVector(0)};

    //const glm::vec3 bias1(20, 10, 0);
    //const glm::vec3 bias2(-10, 0, 0);
    //const glm::vec3 bias3(0, -10, 0);
    //body0.position += bias1;
    //body1.position += bias2;
    //body2.position += bias3;

    body0.position.z = 0;
    body1.position.z = 0;
    body2.position.z = 0;

    //    glm::dvec3 offset = 1/3.0*(body0.position + body1.position + body2.position);
    glm::dvec3 posOffset = body0.position;
    //  glm::dvec3 offset(0);
//    glm::dvec3 velocity = 1/3.0*(body0.velocity + body1.velocity + body2.velocity);
    //    glm::dvec3 velocity(0);

    // Center the system around the origin of coords
    body0.position -= posOffset;
    body1.position -= posOffset;
    body2.position -= posOffset;
    body1.position.y = -1.0;
    body2.position.y = 2.0;

    // Remove system velocity
    //body0.velocity -= velocity;
    //body1.velocity -= velocity;
    //body2.velocity -= velocity;

    return {body0, body1, body2};
}

void OrbitGenerator::generateData()
{
    std::vector<glm::vec3> startingPoints;

    int numLines = 2;

    std::cout << "Generating data..." << std::endl;
	states.clear();
	lines.clear();

#pragma omp parallel
#pragma omp for
    for (int i = 0; i < numLines; ++i) {
        auto tbs = randomSystem();
        auto orbits = solver.computeOrbit(tbs, 8000);
        states.push_back(orbits.first);
        lines.push_back(orbits.second);
    }
}


std::vector<std::vector<float>> OrbitGenerator::computeColors()
{
    std::vector<std::vector<float>> colors;
    for (auto curOrbit : states) {
        std::vector<float> orbitColor;
		auto color = randomVector(1.0);
        for (auto state : curOrbit) {
            // orbitColor.push_back(glm::length(state.body[coloredBody].velocity)*0.01);
            // orbitColor.push_back(state.body[coloredBody].position.x*state.body[coloredBody].position.x*10.0);
            // orbitColor.push_back(state.body[coloredBody].position.y*state.body[coloredBody].position.y*10.0);
            orbitColor.push_back(color.x);
            orbitColor.push_back(color.y);
            orbitColor.push_back(color.z);
        }
        colors.push_back(orbitColor);
    }
    return colors;
}

void OrbitGenerator::updateColors()
{
    auto colors = computeColors();
    // std::cout << "Sending data to renderer." << std::endl;
    // phaseRender->updateData(lines, colors);
}
