#version 330 core
layout (location = 0) in vec3 aPos;
layout (std140) uniform Matrices {
  uniform mat4 view;        // 16x4   // 0
  uniform mat4 projection;  // 16x4   // 64
                                      // total size 128
};

uniform mat4 model;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
}
