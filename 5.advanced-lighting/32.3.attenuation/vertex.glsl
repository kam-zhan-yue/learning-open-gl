#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 view;
uniform mat4 projection;

out V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texture;
} v_out;

void main()
{
  gl_Position = projection * view * vec4(aPos, 1.0);
  v_out.position = aPos;
  v_out.normal = aNormal;
  v_out.texture = aTexCoords;
}
