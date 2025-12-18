#version 330 core

in V_OUT {
  vec2 texCoords;
  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} f_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

out vec4 FragColor;

void main() {    
  float gamma = 2.2f;
  vec3 lightColor = vec3(0.3);
  vec3 color = texture(diffuseMap, f_in.texCoords).rgb;

  // Calculations
  // obtain normal from the normal map in range [0, 1]
  vec3 normal = texture(normalMap, f_in.texCoords).rgb;
  // transform into range [-1, 1]
  normal = normalize(normal * 2.0 - 1.0);
  vec3 lightDir = normalize(f_in.tangentLightPos - f_in.tangentFragPos);
  vec3 viewDir = normalize(f_in.tangentViewPos - f_in.tangentFragPos);

  // Ambient
  vec3 ambient = 0.1 * color; 

  // Diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;

  // Specular
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 8.0);
  vec3 specular = spec * lightColor; // bright white light colour

  vec3 lighting = (ambient + diffuse + specular) * color;
  FragColor = vec4(pow(lighting, vec3(1.0/gamma)), 1.0);
}
