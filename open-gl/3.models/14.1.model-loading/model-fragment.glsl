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

void main()
{    
    FragColor = texture(material.texture_diffuse1, TexCoords);
}

