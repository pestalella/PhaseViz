#version 130

in vec3 vertPos;
in vec3 vertColor;
in vec3 vertNormal;

out vec3 color;

float lambert(vec3 N, vec3 L)
{
  vec3 nrmN = normalize(N);
  vec3 nrmL = normalize(L);
  float result = dot(nrmN, nrmL);
  return max(result, 0.2);
}

void main()
{
  vec3 lightPos = vec3(-10,-10,10);
  color = vertColor;//*lambert(vertNormal, lightPos-vertPos);
}
