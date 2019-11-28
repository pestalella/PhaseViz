#version 330

uniform mat4 modelViewProjMatrix;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 inColor;

out vec3 vertColor;

void main()
{
    vertColor = inColor;
    gl_Position = modelViewProjMatrix * vec4(position, 1.0);
}

