#version 330 core

in vec2 TexCoords;

uniform sampler2D texture1;

out vec4 FragColor;

void main() {
  vec4 texColour = texture(texture1, TexCoords);
  if (texColour.a < 0.2) {
    discard;
  }
  FragColor = texColour;
}
