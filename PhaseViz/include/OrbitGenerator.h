#pragma once

#include "RenderGL.h"

#include <memory>

class OrbitGenerator
{
public:
    void generateData();
	std::vector<std::vector<float>> computeColors();
	void updateColors();
	std::vector<std::vector<float>> orbitLines() const {
		return lines;
	}

private:
    void nextColoredBody() { coloredBody = (coloredBody+1)%3; }

private:
	std::vector<std::vector<float>> lines;
    int coloredBody;
};
