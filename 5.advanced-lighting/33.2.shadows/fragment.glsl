#version 330 core

in V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texCoords;
  vec4 lightSpacePosition;
} f_in;

uniform vec3 viewPos;
uniform vec3 lightPos;

uniform sampler2D texture1;
uniform sampler2D shadowMap;

out vec4 FragColor;

float shadowCalculation() {
  // perform perspective divide
  vec4 lightSpacePosition = f_in.lightSpacePosition;
  vec3 projCoords = lightSpacePosition.xyz / lightSpacePosition.w;

  // transform the NDC coordinates to the range [0, 1] to compare with depth map
  projCoords = projCoords * 0.5 + 0.5;
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  float currentDepth = projCoords.z;

  float shadow = currentDepth > closestDepth ? 1.0 : 0.0;
  return shadow;
}

void main() {    
  float gamma = 2.2;
  vec3 color = texture(texture1, f_in.texCoords).rgb;
  vec3 lightColor = vec3(0.3);

  // Calculations
  vec3 normal = normalize(f_in.normal);
  vec3 lightDir = normalize(lightPos - f_in.position);
  vec3 viewDir = normalize(viewPos - f_in.position);

  // Ambient
  vec3 ambient = 0.05 * color; 

  // Diffuse
  float diff = max(dot(lightDir, normal), 0.0);
  vec3 diffuse = diff * lightColor;

  // Specular
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
  vec3 specular = spec * lightColor; // bright white light colour

  float shadow = shadowCalculation();
  float inverseShadow = 1.0 - shadow;

  vec3 lightning = (ambient + inverseShadow * (diffuse + specular)) * color;
  FragColor = vec4(pow(lightning, vec3(1.0/gamma)), 1.0);
}

