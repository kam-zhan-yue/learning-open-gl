#version 330 core

in vec2 TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main() {
  float depthValue = texture(screenTexture, TexCoords).r;
  FragColor = vec4(vec3(depthValue), 1.0);
}
