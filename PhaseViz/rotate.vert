#version 330

uniform mat4 modelViewProjMatrix;

layout(location = 0) in vec4 position;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;

out vec3 vertPos;
out vec3 vertColor;
out vec3 vertNormal;

void main()
{
	vertColor = inColor;

	mat3 modelViewMatrix = mat3(modelViewProjMatrix);
	mat3 normalMatrix = inverse(modelViewMatrix);
	normalMatrix = transpose(normalMatrix);
	vertNormal = inNormal*normalMatrix;
	vertPos = modelViewMatrix * vec3(position);
	gl_Position = modelViewProjMatrix * position;
}

