#version 330 core

struct Material {
  sampler2D diffuse;
  sampler2D specular;
  float shininess;
};

struct FlashLight {
  vec3 position;
  vec3 direction;
  float cutoff;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

out vec4 FragColor;

uniform Material material;
uniform FlashLight light;
uniform vec3 viewPos;

void main()
{
  // Common calculations
  vec3 lightDir = normalize(light.position - FragPos);
  vec3 textureColour = vec3(texture(material.diffuse, TexCoords));

  // Calculate Ambient Light
  vec3 ambient = textureColour * light.ambient;

  // Calculate Diffuse Light
  vec3 normal = normalize(Normal);
  float diff = max(dot(normal, lightDir), 0.0);
  vec3 diffuse = light.diffuse * diff * textureColour;

  // Calculate Specular Light
  vec3 specularColour = vec3(texture(material.specular, TexCoords));
  vec3 viewDir = normalize(viewPos - FragPos);
  vec3 reflectDir = reflect(-lightDir, normal);
  float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
  vec3 specular = light.specular * (spec * specularColour);

  // Flashlight Logic
  float theta = dot(lightDir, normalize(-light.direction));
  if (theta > light.cutoff) {
    // If we are in the spotlight, do lighting calculations
    vec3 result = diffuse + ambient + specular;
    FragColor = vec4(result, 1.0);
  } else {
    // If we are not in the spotlight, use ambient light
    vec3 result = ambient;
    FragColor = vec4(result, 1.0);
  }
}
