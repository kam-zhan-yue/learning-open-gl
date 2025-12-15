#version 330 core

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
  float shininess;
};

in vec3 FragPos;
in vec3 Normal;

out vec4 FragColor;

uniform Material material;
uniform vec3 lightPos;
uniform vec3 lightColour;
uniform vec3 objectColour;
uniform vec3 viewPos;

void main()
{
  // Calculate Ambient Light
  vec3 ambient = material.ambient * lightColour;

  // Calculate Diffuse Light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(lightPos - FragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = lightColour * (diff * material.diffuse);

  // Calculate Specular Light
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = lightColour * (spec * material.specular);

  // Bring it all together
  vec3 result = diffuse + ambient + specular;
  FragColor = vec4(result, 1.0);
}
