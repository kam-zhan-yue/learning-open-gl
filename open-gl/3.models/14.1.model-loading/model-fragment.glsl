#version 330 core

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

struct SpotLight {
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

struct Material {
  sampler2D texture_specular1;
  sampler2D texture_diffuse1;
  float shininess;
};

out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;


uniform Material material;

#define NUM_POINT_LIGHTS 4
uniform PointLight pointLights[NUM_POINT_LIGHTS];
uniform DirectionalLight directionalLight;

uniform SpotLight spotLight;
uniform vec3 viewPos;

vec3 getAmbient(vec3 ambient, vec3 textureColour) {
  return textureColour * ambient;
}

vec3 getDiffuse(vec3 diffuse, vec3 textureColour, vec3 normal, vec3 lightDir) {
  float diff = max(dot(normal, lightDir), 0.0);
  return diffuse * diff * textureColour;
}

vec3 getSpecular(vec3 specular, vec3 normal, vec3 lightDir, vec3 viewDir) {
  vec3 specularColour = vec3(texture(material.texture_specular1, TexCoords));
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  return specular * (spec * specularColour);
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

void main()
{    
  vec3 result = vec3(0.0);
  vec3 normal = normalize(Normal);
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 textureColour = vec3(texture(material.texture_diffuse1, TexCoords));
  result += calculateDirectionalLight(directionalLight, normal, viewDir, textureColour);

  FragColor = vec4(result, 1.0);
}

