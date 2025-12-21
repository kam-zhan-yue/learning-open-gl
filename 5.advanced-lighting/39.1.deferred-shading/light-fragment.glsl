#version 330 core

layout(location = 0) out vec4 FragColor;

uniform vec3 lightColour;

void main() {
  FragColor = vec4(lightColour, 1.0);
}
