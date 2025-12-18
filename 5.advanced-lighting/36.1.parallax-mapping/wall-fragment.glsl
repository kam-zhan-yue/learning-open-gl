#version 330 core

in V_OUT {
  vec3 position;
  vec2 texCoords;
  vec3 tangentLightPos;
  vec3 tangentViewPos;
  vec3 tangentFragPos;
} f_in;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D depthMap;
uniform float depthScale;

out vec4 FragColor;

vec2 parallaxMapping(vec2 texCoords, vec3 viewDir) {
  // Get the position of the depth map at the fragment position
  float depth = texture(depthMap, texCoords).r;
  vec2 p = viewDir.xy / viewDir.z * (depth * depthScale);
  return texCoords - p;
}


void main() {    
  float gamma = 2.2f;
  vec3 lightColor = vec3(0.3);

  // Calculations
  vec3 lightDir = normalize(f_in.tangentLightPos - f_in.tangentFragPos);
  vec3 viewDir = normalize(f_in.tangentViewPos - f_in.tangentFragPos);
  vec2 texCoords = parallaxMapping(f_in.texCoords, viewDir); // adjust the texCoords based on parallax mapping

  // Discard if oversampled outside the texture
  if (texCoords.x > 1.0 || texCoords.y > 1.0 || texCoords.x < 0.0 || texCoords.y < 0.0)
    discard;

  // sample textures with the new texture coordinates
  // obtain normal from the normal map in range [0, 1]
  vec3 normal = texture(normalMap, texCoords).rgb;
  // transform into range [-1, 1]
  normal = normalize(normal * 2.0 - 1.0);
  vec3 color = texture(diffuseMap, texCoords).rgb;

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
