#version 330 core

layout (location = 0) in vec2 aPosition;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aOffset;

out V_OUT {
  vec3 colour;
} v_out;

void main() {
  vec2 pos = aPosition * (gl_InstanceID / 100.0);
  gl_Position = vec4(pos + aOffset, 0.0, 1.0);
  v_out.colour = aColour;
}
