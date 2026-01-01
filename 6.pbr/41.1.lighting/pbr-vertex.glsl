#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out V_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
} v_out;

uniform bool invertedNormals;

void main()
{
  vec4 viewPos = view * model * vec4(aPos, 1.0);
  gl_Position = projection * viewPos;
  v_out.texCoords = aTexCoords;
  v_out.position = viewPos.xyz;
  mat3 normalMatrix = mat3(transpose(inverse(view * model)));
  v_out.normal = normalize(normalMatrix * (invertedNormals ? -aNormal : aNormal));
}
