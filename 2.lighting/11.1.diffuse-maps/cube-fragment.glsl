#version 330 core

struct Material {
  sampler2D diffuse;
  vec3 specular;
  float shininess;
};

struct Light {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform Light light;
uniform vec3 viewPos;

void main()
{
  vec3 textureColour = vec3(texture(material.diffuse, TexCoords));
  // Calculate Ambient Light
  vec3 ambient = textureColour * light.ambient;

  // Calculate Diffuse Light
  vec3 normal = normalize(Normal);
  vec3 lightDir = normalize(light.position - FragPos);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * textureColour;

  // Calculate Specular Light
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * material.specular);

  // Bring it all together
  vec3 result = diffuse + ambient + specular;
  FragColor = vec4(result, 1.0);
}
