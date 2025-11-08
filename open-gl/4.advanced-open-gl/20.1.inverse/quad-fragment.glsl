#version 330 core

in vec2 TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main() {
  FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoords)), 1.0);
}
