#version 330 core

in vec2 texCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;

out vec4 FragColor;

struct Light {
  vec3 position;
  vec3 colour;
};

const int NUM_LIGHTS = 32;
uniform Light lights[NUM_LIGHTS];

void main() {
  vec3 position = texture(positionBuffer, texCoords).rgb;
  vec3 normal = texture(normalBuffer, texCoords).rgb;
  vec3 colour = texture(albedoBuffer, texCoords).rgb;
  float specular = texture(albedoBuffer, texCoords).a;

  vec3 ambient = 0.1 * colour;
  vec3 diffuse = vec3(0.0);

  for (int i=0; i<NUM_LIGHTS; ++i) {
    vec3 lightDir = normalize(lights[i].position - position);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 result = lights[i].colour * diff * colour;
    float distance = length(lights[i].position - position);
    result *= 1.0 / (distance * distance);
    diffuse += result;
  }
  vec3 lighting = ambient + diffuse;

  FragColor = vec4(lighting, 1.0);
}
