#version 330 core

in vec2 texCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;
uniform sampler2D ssaoBuffer;

struct Light {
  vec3 position;
  vec3 colour;
  float linear;
  float quadratic;
};

uniform Light light;

out vec4 FragColor;

void main() {
  vec3 position = texture(positionBuffer, texCoords).rgb;
  vec3 normal = texture(normalBuffer, texCoords).rgb;
  vec3 colour = texture(albedoBuffer, texCoords).rgb;
  float ssao = texture(ssaoBuffer, texCoords).r;

  // Calculations
  vec3 viewDir = normalize(-position);
  vec3 lightDir = normalize(light.position - position);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
  float dist = length(light.position - position);

  // Lighting
  vec3 ambient = vec3(0.3 * colour * ssao);
  vec3 diffuse = max(dot(normal, lightDir), 0.0) * colour * light.colour;
  vec3 specular = light.colour * spec;
  float attenuation = 1.0 / (1.0 + light.linear * dist + light.quadratic * dist * dist);
  diffuse *= attenuation;
  specular *= attenuation;
  vec3 lighting = ambient + diffuse + specular;

  FragColor = vec4(lighting, 1.0);
}
