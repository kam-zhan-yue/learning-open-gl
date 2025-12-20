#version 330 core

struct Light {
  vec3 position;
  vec3 colour;
};

in V_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
} f_in;

uniform Light[16] lights;
uniform sampler2D diffuseTexture;

out vec4 FragColor;

void main() {
  vec3 colour = texture(diffuseTexture, f_in.texCoords).rgb;

  // ambient
  vec3 ambient = 0.01 * colour;
  vec3 diffuse = vec3(0.0);

  for (int i=0; i<16; i++) {
    // diffuse
    vec3 lightDir = normalize(lights[i].position - f_in.position);
    float diff = max(dot(lightDir, f_in.normal), 0.0);
    vec3 result = lights[i].colour * diff * colour;
    // attenuation
    float distance = length(lights[i].position - f_in.position);
    result *= 1.0 / (distance * distance);
    diffuse += result;
  }

  vec3 lighting = ambient + diffuse;
  FragColor = vec4(lighting, 1.0);
}
