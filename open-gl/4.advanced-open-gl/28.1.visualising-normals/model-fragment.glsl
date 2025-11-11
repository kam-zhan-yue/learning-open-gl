#version 330 core

struct Material {
  sampler2D texture_specular1;
  sampler2D texture_diffuse1;
  float shininess;
};

out vec4 FragColor;

in V_OUT {
  vec2 TexCoords;
} f_in;


uniform Material material;

void main() {    
  vec3 textureColour = vec3(texture(material.texture_diffuse1, f_in.TexCoords));
  FragColor = vec4(textureColour, 1.0);
}

