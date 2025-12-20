#version 330 core

in vec2 texCoords;

uniform sampler2D colorBuffer;

out vec4 FragColor;

void main() {
  const float gamma = 2.2f;
  vec3 hdr = texture(colorBuffer, texCoords).rgb;

  // reinhard tone mapping
  vec3 mapped = hdr / (hdr + vec3(1.0));
  mapped = pow(mapped, vec3(1.0 / gamma));
  FragColor = vec4(mapped, 1.0);
}
