#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aModel;

uniform mat4 view;
uniform mat4 projection;

out V_OUT {
  vec3 normal;
  vec3 position;
  vec2 tex;
} v_out;

void main()
{
  gl_Position = projection * view * aModel * vec4(aPos, 1.0);
  v_out.position = vec3(aModel * vec4(aPos, 1.0));
  v_out.normal = mat3(transpose(inverse(aModel))) * aNormal;
  v_out.tex = aTexCoords;
}
