#include "utils.h"

glm::dvec3 randomVector(double scale)
{
    return glm::dvec3((rand() / (RAND_MAX + 1.0) - 0.5)*2*scale,
        (rand() / (RAND_MAX + 1.0) - 0.5)*2*scale,
        (rand() / (RAND_MAX + 1.0) - 0.5)*2*scale);
}
