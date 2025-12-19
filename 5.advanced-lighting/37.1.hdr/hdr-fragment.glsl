#version 330 core

in vec2 texCoords;

uniform sampler2D colorBuffer;

out vec4 FragColor;

void main() {
  FragColor = texture(colorBuffer, texCoords);
}
