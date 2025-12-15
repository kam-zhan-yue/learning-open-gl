#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main() {
  TexCoords = aPos;
  vec4 pos = projection * view * vec4(aPos, 1.0);
  // Trick OpenGL into thinking that our skybox has a z of 1 (furthest depth value)
  // After perspective division, all values are divided by w
  gl_Position = pos.xyww;
}
