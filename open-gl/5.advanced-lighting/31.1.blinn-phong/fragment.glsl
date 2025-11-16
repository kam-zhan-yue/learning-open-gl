#version 330 core

in V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texture;
} f_in;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform sampler2D texture1;
uniform bool blinn;

out vec4 FragColor;

void main() {    
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
  float spec = 0.0;
  if (blinn) {
    vec3 halfwayDir = normalize(lightDir + viewDir);
    spec = pow(max(dot(normal, halfwayDir), 0.0), 0.5);
  } else {
    vec3 reflectDir = reflect(-lightDir, normal);
    spec = pow(max(dot(viewDir, reflectDir), 0.0), 0.5);
  }
  vec3 specular = vec3(0.3) * spec; // bright white light colour

  FragColor = vec4(ambient + diffuse + specular, 1.0);
}

