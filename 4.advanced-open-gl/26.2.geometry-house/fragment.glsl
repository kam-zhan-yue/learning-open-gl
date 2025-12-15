#version 330 core

in GS_OUT {
  vec3 colour;
} f_in;

out vec4 FragColor;

void main() {
  FragColor = vec4(f_in.colour, 1.0);
}
