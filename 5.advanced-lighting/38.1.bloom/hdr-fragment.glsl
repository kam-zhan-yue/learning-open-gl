#version 330 core

in vec2 texCoords;

uniform sampler2D colorBuffer;
uniform sampler2D blurBuffer;
uniform float exposure;

out vec4 FragColor;

void main() {
  const float gamma = 2.2f;
  vec3 hdr = texture(colorBuffer, texCoords).rgb;
  vec3 blur = texture(blurBuffer, texCoords).rgb;
  hdr += blur; // additive blending

  // reinhard tone mapping
  vec3 mapped = vec3(1.0) - exp(-hdr * exposure);
  mapped = pow(mapped, vec3(1.0 / gamma));
  FragColor = vec4(mapped, 1.0);
}
