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
  float shadow = 0.0;
  float bias = 0.05;
  float samples = 4.0;
  float offset = 0.1;

  // get the direction of the frag pos from the light position
  vec3 fragToLight = f_in.position - lightPos;
  float currentDepth = length(fragToLight);

  // Method 1: Sample everything around the axex. With samples at 4.0, this runs a total of 64 times (per fragment)!
  // for (float x = -offset; x < offset; x += offset / (samples * 0.5)) {
  //   for (float y = -offset; y < offset; y += offset / (samples * 0.5)) {
  //     for (float z = -offset; z < offset; z += offset / (samples * 0.5)) {
  //       float closestDepth = texture(shadowMap, fragToLight + vec3(x, y, z)).r;
  //       closestDepth *= farPlane;
  //       if (currentDepth - bias > closestDepth)
  //         shadow += 1.0;
  //     }
  //   }
  // }
  //
  // shadow /= (samples * samples * samples);

  // Method 2: Sample in specific offset directions
  vec3 sampleOffsetDirections[20] = vec3[]
  (
    vec3( 1, 1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1, 1,  1),
    vec3( 1, 1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1, 1, -1),
    vec3( 1, 1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1, 1,  0),
    vec3( 1, 0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1, 0, -1),
    vec3( 0, 1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0, 1, -1)
  );
  float viewDistance = length(viewPos - f_in.position);
  float diskRadius = (1.0 + (viewDistance / farPlane)) / 25.0;

  for (int i = 0; i < 20; ++i) {
    float closestDepth = texture(shadowMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
    closestDepth *= farPlane;
    if (currentDepth - bias > closestDepth)
      shadow += 1.0;
  }

  shadow /= 20.0;

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

