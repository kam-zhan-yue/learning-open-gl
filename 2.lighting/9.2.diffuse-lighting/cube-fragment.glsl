#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 objectColour;

void main()
{
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColour;

  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColour;
  vec3 result = (diffuse + ambient) * objectColour;
  FragColor = vec4(result, 1.0);
}
