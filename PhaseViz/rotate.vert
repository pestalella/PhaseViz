#version 410
 
//layout (std140) 
uniform mat4 modelViewProjMatrix;

in vec3 position;
in vec3 inColor;
 
out vec3 vertColor;
 
void main()
{
    vertColor = inColor;
    //vertColor = vec3(1, 0, 1);
    gl_Position = modelViewProjMatrix * vec4(position, 1.0);
}