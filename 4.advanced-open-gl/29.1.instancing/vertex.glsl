#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec3 aColour;

out V_OUT {
  vec3 colour;
} v_out;

uniform vec2 offsets[100];

void main() {
  vec2 offset = offsets[gl_InstanceID];
  gl_Position = vec4(aPosition + offset, 0.0, 1.0);
  v_out.colour = aColour;
}
