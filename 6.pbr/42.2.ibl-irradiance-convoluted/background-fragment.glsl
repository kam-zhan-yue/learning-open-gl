#version 330 core

in vec3 localPos;
out vec4 FragColor;
uniform samplerCube environmentCubemap;

void main() {
  vec3 colour = texture(environmentCubemap, localPos).rgb;
  colour = colour / (colour + vec3(1.0));
  colour = pow(colour, vec3(1.0 / 2.2));
  FragColor = vec4(colour, 1.0);
}
