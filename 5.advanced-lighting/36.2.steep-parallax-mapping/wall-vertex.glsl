#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

out V_OUT {
  vec2 texCoords;
  vec3 position;
  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} v_out;

void main() {
  gl_Position = projection * view * model * vec4(aPos, 1.0);
  v_out.position = vec3(model * vec4(aPos, 1.0));
  v_out.texCoords = aTexCoords;

  // Gram-Schmidt method
  mat3 normalMatrix = transpose(inverse(mat3(model)));
  vec3 T = normalize(normalMatrix * aTangent);
  vec3 N = normalize(normalMatrix * aNormal);
  // re-orthogonalise T with respect to N
  T = normalize(T - dot(T, N) * N);
  // retrieve B with the cross product of T and N
  vec3 B = cross(N, T);

  mat3 TBN = transpose(mat3(T, B, N));
  v_out.tangentLightPos = TBN * lightPos;
  v_out.tangentViewPos = TBN * viewPos;
  v_out.tangentFragPos = TBN * v_out.position;
}
