#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 5) out;

in vec3 VertexColour[];
// input variables in geometry shaders are always arrays
in VS_OUT {
  vec3 colour;
} gs_in[];

out GS_OUT {
  vec3 colour;
} gs_out;

void build_house(vec4 position) {
  gs_out.colour = gs_in[0].colour;

  gl_Position = position + vec4(-0.2, -0.2, 0.0, 0.0); // bottom-left
  EmitVertex();
  gl_Position = position + vec4(0.2, -0.2, 0.0, 0.0); // bottom-right
  EmitVertex();
  gl_Position = position + vec4(-0.2, 0.2, 0.0, 0.0); // top-left
  EmitVertex();
  gl_Position = position + vec4(0.2, 0.2, 0.0, 0.0); // top-right
  EmitVertex();
  gl_Position = position + vec4(0, 0.4, 0.0, 0.0); // top
  gs_out.colour = vec3(1.0);
  EmitVertex();

  EndPrimitive();
}


void main() {
  build_house(gl_in[0].gl_Position);
}
