#version 330 core

in vec3 Position;
in vec3 Normal;

uniform vec3 cameraPos;
uniform samplerCube environment;

out vec4 FragColor;

void main() {
  vec3 viewDir = normalize(Position - cameraPos);
  // reflection
  vec3 reflection = reflect(viewDir, normalize(Normal));
  FragColor = vec4(texture(environment, reflection).rgb, 1.0);

  // refraction
  float ratio = 1.00 / 1.52;
  vec3 refraction = refract(viewDir, normalize(Normal), ratio);
  FragColor = vec4(texture(environment, refraction).rgb, 1.0);
}
