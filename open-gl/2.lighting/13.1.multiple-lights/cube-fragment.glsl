#version 330 core

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct DirectionalLight {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct PointLight {
  vec3 position;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

struct FlashLight {
  vec3 position;
  vec3 direction;
  float inner;
  float outer;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;

  float constant;
  float linear;
  float quadratic;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;

#define NUM_POINT_LIGHTS 4
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform DirectionalLight directionalLight;

uniform FlashLight light;
uniform vec3 viewPos;

vec3 getAmbient(vec3 ambient, vec3 textureColour) {
  return textureColour * ambient;
}

vec3 getDiffuse(vec3 diffuse, vec3 textureColour, vec3 normal, vec3 lightDir) {
  float diff = max(dot(normal, lightDir), 0.0);
  return diffuse * diff * textureColour;
}

vec3 getSpecular(vec3 specular, vec3 normal, vec3 lightDir, vec3 viewDir) {
  vec3 specularColour = vec3(texture(material.specular, TexCoords));
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  return specular * (spec * specularColour);
}

float getAttenuation(vec3 position, float constant, float linear, float quadratic) {
  float distance = length(position - FragPos);
  return 1.0 / (constant + linear * distance + quadratic * (distance * distance));
}

vec3 calculateDirectionalLight(DirectionalLight light, vec3 normal, vec3 viewDir, vec3 textureColour) {
  // Directional light has its own direction
  vec3 lightDir = normalize(-light.direction);
  vec3 ambient = getAmbient(light.ambient, textureColour);
  vec3 diffuse = getDiffuse(light.diffuse, textureColour, normal, lightDir);
  vec3 specular = getSpecular(light.specular, normal, lightDir, viewDir);
  vec3 result = diffuse + ambient + specular;
  return result;
}

vec3 calculatePointLight(PointLight light, vec3 normal, vec3 viewDir, vec3 textureColour) {
  // Point light calculates direction based on positional difference
  vec3 lightDir = normalize(light.position - FragPos);

  vec3 ambient = getAmbient(light.ambient, textureColour);
  vec3 diffuse = getDiffuse(light.diffuse, textureColour, normal, lightDir);
  vec3 specular = getSpecular(light.specular, normal, lightDir, viewDir);

  float attenuation = getAttenuation(light.position, light.constant, light.linear, light.quadratic);
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;
  return ambient + diffuse + specular;
}

void main()
{
  // Common calculations
  vec3 lightDir = normalize(light.position - FragPos);
  vec3 textureColour = vec3(texture(material.diffuse, TexCoords));

  // Calculate Ambient Light
  vec3 ambient = textureColour * light.ambient;

  // Calculate Diffuse Light
  vec3 normal = normalize(Normal);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * textureColour;

  // Calculate Specular Light
  vec3 specularColour = vec3(texture(material.specular, TexCoords));
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * specularColour);

  // Spotlight
  float theta = dot(lightDir, normalize(-light.direction));
  float epsilon = light.inner - light.outer;
  float intensity = clamp((theta - light.outer) / epsilon, 0.0, 1.0);
  
  diffuse *= intensity;
  specular *= intensity;

  // Attenuation
  float distance = length(light.position - FragPos);
  float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));
  ambient *= attenuation;
  diffuse *= attenuation;
  specular *= attenuation;

  vec3 result = vec3(0.0);
  result += calculateDirectionalLight(directionalLight, normal, viewDir, textureColour);
  FragColor = vec4(result, 1.0);
}
