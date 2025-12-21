#version 330 core

// out vec4 FragColor;
layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec4 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

struct Material {
  sampler2D texture_specular1;
  sampler2D texture_diffuse1;
  float shininess;
};

in V_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
} f_in;

uniform Material material;

void main() {    
  gPosition = vec4(f_in.position, 1.0);
  gNormal = vec4(f_in.normal, 1.0);
  gAlbedoSpec.rgb = texture(material.texture_diffuse1, f_in.texCoords).rgb;
  gAlbedoSpec.a = texture(material.texture_specular1, f_in.texCoords).r;
}
