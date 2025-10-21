#version 330 core

out vec4 FragColor;

uniform vec3 lightColour;
uniform vec3 objectColour;

void main()
{
  float ambientStrength = 0.1;
  vec3 ambientLight = ambientStrength * lightColour;
  vec3 result = ambientLight * objectColour;
  FragColor = vec4(result, 1.0);
}
