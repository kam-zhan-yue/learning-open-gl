#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texCoords;
} v_out;

uniform bool reverse_normals;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  // a slight hack to make sure the outer large cube displays lighting from the inside
  if (reverse_normals) {
    v_out.normal = transpose(inverse(mat3(model))) * (-1.0 * aNormal);
  } else { 
    v_out.normal = transpose(inverse(mat3(model))) * aNormal;
  }
  v_out.position = vec3(model * vec4(aPos, 1.0));
  v_out.texCoords = aTexCoords;
}
