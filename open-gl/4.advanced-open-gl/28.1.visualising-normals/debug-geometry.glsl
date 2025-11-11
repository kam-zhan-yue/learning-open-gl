#version 330 core

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in V_OUT {
  // vec3 Normal;
  vec2 TexCoords;
} g_in[];

out G_OUT {
  vec2 TexCoords;
} g_out;

vec3 getNormal() {
  vec3 a = vec3(gl_in[0].gl_Position) - vec3(gl_in[1].gl_Position);
  vec3 b = vec3(gl_in[1].gl_Position) - vec3(gl_in[2].gl_Position);
  return normalize(cross(a, b));
}

void main() {
  vec3 normal = getNormal();
  gl_Position = gl_in[0].gl_Position, normal;
  g_out.TexCoords = g_in[0].TexCoords;
  EmitVertex();

  gl_Position = gl_in[1].gl_Position, normal;
  g_out.TexCoords = g_in[1].TexCoords;
  EmitVertex();

  gl_Position = gl_in[2].gl_Position, normal;
  g_out.TexCoords = g_in[2].TexCoords;
  EmitVertex();

  EndPrimitive();
}
