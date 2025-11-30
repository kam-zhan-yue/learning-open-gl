#version 330 core

in V_OUT {
  vec3 normal;
  vec3 position;
  vec2 texCoords;
} f_in;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform float farPlane;

uniform sampler2D texture1;
uniform samplerCube shadowMap;

out vec4 FragColor;

float shadowCalculation() {
  // get the direction of the frag pos from the light position
  vec3 fragToLight = f_in.position - lightPos;
  // sample the shadow cubemap
  float closestDepth = texture(shadowMap, fragToLight).r;
  // closest depth is [0, 1] to we transform it to [0, farPlane]
  closestDepth *= farPlane;
  // retrieve the depth value between the current fragment and the light source
  float currentDepth = length(fragToLight);
  // compare both depth values to see which is closer than the other and include shadow bias
  float bias = 0.05;
  float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
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

  vec3 lighting = (ambient + inverseShadow * (diffuse + specular)) * color;
  FragColor = vec4(pow(lighting, vec3(1.0/gamma)), 1.0);

  // DEBUGGING PURPOSES, COMMENT WHEN DONE
  // get the direction of the frag pos from the light position
  // vec3 fragToLight = f_in.position - lightPos;
  // // sample the shadow cubemap
  // float closestDepth = texture(shadowMap, fragToLight).r;
  // FragColor = vec4(vec3(closestDepth), 1.0);
}

