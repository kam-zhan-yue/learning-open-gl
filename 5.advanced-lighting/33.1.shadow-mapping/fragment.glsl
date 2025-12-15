#version 330 core

in V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texture;
} f_in;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform sampler2D texture1;

out vec4 FragColor;

void main() {    
  float gamma = 2.2;
  // Loading the textures as GL_SRGB would render this unnecessary
  // vec3 color = pow(texture(texture1, f_in.texture).rgb, vec3(gamma));
  vec3 color = texture(texture1, f_in.texture).rgb;

  // Calculations
  vec3 normal = normalize(f_in.normal);
  vec3 lightDir = normalize(lightPos - f_in.position);
  vec3 viewDir = normalize(viewPos - f_in.position);

  // Ambient
  vec3 ambient = 0.05 * color; 

  // Diffuse
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = diff * color;

  // Specular
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
  vec3 specular = vec3(0.3) * spec; // bright white light colour

  vec3 result = vec3(ambient + diffuse + specular);
  FragColor = vec4(pow(result, vec3(1.0/gamma)), 1.0);
}

