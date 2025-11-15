#version 330 core

uniform sampler2D framebuffer;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
  FragColor = texture(framebuffer, TexCoords);
}
