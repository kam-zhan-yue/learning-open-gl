#version 330 core

in vec2 texCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D albedoBuffer;
uniform sampler2D ssaoBuffer;

out vec4 FragColor;

void main() {
  float ssao = texture(ssaoBuffer, texCoords).r;
  FragColor = vec4(vec3(ssao), 1.0);

  // vec3 position = texture(positionBuffer, texCoords).rgb;
  // vec3 normal = texture(normalBuffer, texCoords).rgb;
  // vec3 colour = texture(albedoBuffer, texCoords).rgb;
  // float specular = texture(albedoBuffer, texCoords).a;
  // vec3 ambient = 0.1 * colour;
  // vec3 diffuse = vec3(0.0);
  // vec3 lighting = ambient + diffuse;
  //
  // FragColor = vec4(lighting, 1.0);
}
