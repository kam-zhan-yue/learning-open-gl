#version 330 core

struct Material {
  sampler2D texture_specular1;
  sampler2D texture_diffuse1;
  float shininess;
};

out vec4 FragColor;

in V_OUT {
  vec3 normal;
  vec3 position;
  vec2 tex;
} f_in;

uniform Material material;

void main() {    
  vec3 textureColour = vec3(texture(material.texture_diffuse1, f_in.tex));
  FragColor = vec4(textureColour, 1.0);
}

