#version 330 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

in V_OUT {
  vec3 Normal;
} g_in[];

uniform mat4 projection;

const float MAGNITUDE = 0.4;

void generateLine(int index) {
  vec4 vertexPos = gl_in[index].gl_Position;

  // Origin Point
  gl_Position = projection * vertexPos;
  EmitVertex();

  // Origin Point + Normal Vector
  gl_Position = projection * (vertexPos + vec4(g_in[index].Normal, 0.0) * MAGNITUDE);
  EmitVertex();

  EndPrimitive();
}

void main() {
  generateLine(0);
  generateLine(1);
  generateLine(2);
}
