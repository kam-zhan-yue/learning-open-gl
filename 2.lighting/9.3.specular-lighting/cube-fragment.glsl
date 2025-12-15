#version 330 core

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 objectColour;
uniform vec3 viewPos;

void main()
{
  // Calculate Ambient Light
  float ambientStrength = 0.1;
  vec3 ambient = ambientStrength * lightColour;

  // Calculate Diffuse Light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * lightColour;

  // Calculate Specular Light
  float specularStrength = 0.5;
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
  vec3 specular = specularStrength * spec * lightColour;

  // Bring it all together
  vec3 result = (diffuse + ambient + specular) * objectColour;
  FragColor = vec4(result, 1.0);
}
