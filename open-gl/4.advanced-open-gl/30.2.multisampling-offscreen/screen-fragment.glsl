#version 330 core

uniform sampler2D framebuffer;

in vec2 TexCoords;
out vec4 FragColor;

void main() {
  vec4 tex = texture(framebuffer, TexCoords);
  float gray = (tex.r + tex.g + tex.b) / 3.0;
  FragColor = vec4(vec3(gray), tex.a);
}
