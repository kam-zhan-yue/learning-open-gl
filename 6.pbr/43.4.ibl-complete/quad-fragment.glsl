#version 330 core

in vec2 texCoords;
out vec4 FragColor;

uniform sampler2D image;

void main() {
  vec2 colour = texture(image, texCoords).rg;
  FragColor = vec4(colour, 0.0, 1.0);
}
