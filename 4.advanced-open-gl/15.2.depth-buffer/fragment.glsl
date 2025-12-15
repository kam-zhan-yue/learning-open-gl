#version 330 core

in vec2 TexCoords;

uniform float near;
uniform float far;
uniform sampler2D texture1;

out vec4 FragColor;

float linearizeDepth(float depth) {
  float ndc = 2.0 * depth - 1.0;
  return (2.0 * near * far) / (far + near - ndc * (far - near));
}

void main() {
  float linearDepth = linearizeDepth(gl_FragCoord.z) / far;
  FragColor = vec4(vec3(linearDepth), 1.0);
}
