#version 330 core

in vec3 vertColor;
out vec3 color;

void main()
{
  color = vertColor;
//  color = vec3(1.0, 1.0, 1.0);
}