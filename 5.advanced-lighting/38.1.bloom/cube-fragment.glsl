#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

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

  // if the fragment output is brighter than the threshold, then output the brightness colour
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  BrightColor = brightness > 1.0 ? vec4(FragColor.rgb, 1.0) : vec4(vec3(0.0), 1.0);

  // FragColor = BrightColor;
  // BrightColor = vec4(lighting, 1.0);
}
