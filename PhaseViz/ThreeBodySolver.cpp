#include "ThreeBodySolver.h"

#include <numeric>

Projection::Projection() :
    rowX(18), rowY(18), rowZ(18)
{
    createMatrix();
}

void Projection::createMatrix()
{
    for (int i = 0; i < 18; ++i) {
        rowX[i] = rand() / (RAND_MAX + 1.0);
        rowY[i] = rand() / (RAND_MAX + 1.0);
        rowZ[i] = rand() / (RAND_MAX + 1.0);
    }
    float sX = 1.0 / std::accumulate(rowX.begin(), rowX.end(), 0.0);
    float sY = 1.0 / std::accumulate(rowY.begin(), rowY.end(), 0.0);
    float sZ = 1.0 / std::accumulate(rowZ.begin(), rowZ.end(), 0.0);

    for (int i = 0; i < 18; ++i) {
        rowX[i] *= sX;
        rowY[i] *= sY;
        rowZ[i] *= sZ;
    }
}

glm::vec3 Projection::phaseSpaceToVizSpace(Body const& b1, Body const& b2, Body const& b3)
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

    float x = std::inner_product(std::begin(phaseSpacePoint), std::end(phaseSpacePoint), std::begin(rowX), 0.0);
    float y = std::inner_product(std::begin(phaseSpacePoint), std::end(phaseSpacePoint), std::begin(rowY), 0.0);
    float z = std::inner_product(std::begin(phaseSpacePoint), std::end(phaseSpacePoint), std::begin(rowZ), 0.0);

    return glm::vec3(x, y, z);
}
glm::vec3 randomVector(float scale=1.0) 
{
    return glm::vec3(
        (rand()/(RAND_MAX + 1.0) - 0.5) * scale,
        (rand()/(RAND_MAX + 1.0) - 0.5) * scale,
        (rand()/(RAND_MAX + 1.0) - 0.5) * scale);
}


Body randomBody()
{
    return { randomVector(0.3), glm::vec3(0)/*randomVector(100.0)*/ };
}

SystemAccels computeAccelerations(Body const &b1, Body const& b2, Body const& b3)
{
    glm::vec3 dir12 = b2.position - b1.position;
    glm::vec3 dir13 = b3.position - b1.position;
    glm::vec3 dir23 = b3.position - b2.position;
    SystemAccels result;
    result.a1 = dir12 * (float)( 1.0 / pow(dir12.length(), 3)) + dir13 * (float)( 1.0 / pow(dir13.length(), 3));
    result.a2 = dir23 * (float)( 1.0 / pow(dir23.length(), 3)) + dir12 * (float)(-1.0 / pow(dir12.length(), 3));
    result.a3 = dir13 * (float)(-1.0 / pow(dir13.length(), 3)) + dir23 * (float)(-1.0 / pow(dir23.length(), 3));
    return result;
}

ThreeBodySolver::ThreeBodySolver()
{
}

std::vector<std::vector<float>>  ThreeBodySolver::randomSolution(int numPoints)
{
    std::vector<float> orbitVertices;
    //std::vector<float> orbitVertices1;
    //std::vector<float> orbitVertices2;
    //std::vector<float> orbitVertices3;

    Body b1 = randomBody(),
         b2 = randomBody(), 
         b3 = randomBody();
 
    const glm::vec3 bias1(1, -1, 0);
    const glm::vec3 bias2(2, 0, 0);
    const glm::vec3 bias3(1, 1, 0);
    b1.position += bias1;
    b2.position += bias2;
    b3.position += bias3;

    glm::vec3 center = 1 / 3.0f * (b1.position + b2.position + b3.position);
//    glm::vec3 velocity = 1 / 3.0f * (b1.velocity + b2.velocity + b3.velocity);
//    glm::vec3 center(0);
    glm::vec3 velocity(0);

    // Center the system around the origin of coords
    b1.position -= center;
    b2.position -= center;
    b3.position -= center;
    // Remove system velocity
    b1.velocity -= velocity;
    b2.velocity -= velocity;
    b3.velocity -= velocity;

    float tStep = 0.5;

    int numSteps = 0;
    int numVerts = 0;

    //for (int i = 0; i < numSteps; ++i) {
    while (numVerts < numPoints) {
        auto accels = computeAccelerations(b1, b2, b3);
        b1.position += tStep * (b1.velocity + tStep / 2 * accels.a1);
        b2.position += tStep * (b2.velocity + tStep / 2 * accels.a2);
        b3.position += tStep * (b3.velocity + tStep / 2 * accels.a3);
        auto newAccels = computeAccelerations(b1, b2, b3);
        b1.velocity += tStep / 2 * (accels.a1 + newAccels.a1);
        b2.velocity += tStep / 2 * (accels.a2 + newAccels.a2);
        b3.velocity += tStep / 2 * (accels.a3 + newAccels.a3);

        numSteps++;
//        if (numSteps % 10 == 1) {
            auto projected = p.phaseSpaceToVizSpace(b1, b2, b3);
            orbitVertices.push_back(projected[0]);
            orbitVertices.push_back(projected[1]);
            orbitVertices.push_back(projected[2]);
            numVerts++;
//        }

        //orbitVertices1.push_back(b1.position[0]);
        //orbitVertices1.push_back(b1.position[1]);
        //orbitVertices1.push_back(b1.position[2]);

        //orbitVertices2.push_back(b2.position[0]);
        //orbitVertices2.push_back(b2.position[1]);
        //orbitVertices2.push_back(b2.position[2]);

        //orbitVertices3.push_back(b3.position[0]);
        //orbitVertices3.push_back(b3.position[1]);
        //orbitVertices3.push_back(b3.position[2]);

    }

    return std::vector<std::vector<float>>({orbitVertices});
}
