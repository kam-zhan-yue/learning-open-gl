#version 330 core

in vec2 TexCoords;

uniform sampler2D screenTexture;

out vec4 FragColor;

void main() {
  vec4 textureColour = texture(screenTexture, TexCoords);
  float average = (textureColour.r + textureColour.g + textureColour.b) / 3.0;
  FragColor = vec4(vec3(average), 1.0);
}
