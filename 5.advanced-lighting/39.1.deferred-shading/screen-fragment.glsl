#version 330 core

in vec2 texCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;

out vec4 FragColor;

void main() {
  vec3 position = texture(positionBuffer, texCoords).rgb;
  vec3 normal = texture(normalBuffer, texCoords).rgb;
  vec3 colour = texture(albedoBuffer, texCoords).rgb;
  float specular = texture(albedoBuffer, texCoords).a;
  FragColor = vec4(normal, 1.0);
}
