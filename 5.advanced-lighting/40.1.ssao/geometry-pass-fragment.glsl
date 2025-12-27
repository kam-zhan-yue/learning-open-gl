#version 330 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gAlbedoSpec;

in V_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
} f_in;

void main() {
  gPosition = f_in.position;
  gNormal = f_in.normal;
  gAlbedoSpec = vec3(0.95);
}
