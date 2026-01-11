#version 330 core

out vec4 FragColor;

struct Light {
  vec3 position;
  vec3 colour;
};

in V_OUT {
  vec3 position;
  vec3 normal;
  vec2 texCoords;
} f_in;

uniform vec3 camPos;
uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ambientOcclusion;

uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

#define NUM_LIGHTS 4
uniform Light lights[NUM_LIGHTS];

const float PI = 3.14159265359;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
  return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
  return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float NdotH = max(dot(N, H), 0.0);
  float NdotH2 = NdotH * NdotH;

  float numerator = a2;
  float denominator = NdotH * (a2 - 1.0) + 1.0;
  denominator = PI * denominator * denominator;
  return numerator / denominator;
}

float geometrySchlickGGX(float NdotV, float roughness) {
  float r = (roughness + 1.0);
  float k = (r * r) / 8.0;
  float numerator = NdotV;
  float denominator = NdotV * (1 - k) + k;
  return numerator / denominator;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
  float NdotV = max(dot(N, V), 0.0);
  float NdotL = max(dot(N, L), 0.0);
  float ggx1 = geometrySchlickGGX(NdotV, roughness);
  float ggx2 = geometrySchlickGGX(NdotL, roughness);
  return ggx1 * ggx2;
}

void main() {
  vec3 N = f_in.normal;
  vec3 V = normalize(camPos - f_in.position);

  // base reflectivity (assume 0.04 for dielectrics)
  vec3 F0 = vec3(0.04);
  F0 = mix(F0, albedo, metallic);

  // reflectance equation
  vec3 Lo = vec3(0.0);
  for (int i = 0; i < NUM_LIGHTS; ++i) {
    // calculate per-light radiance
    vec3 L = normalize(lights[i].position - f_in.position);
    vec3 H = normalize(V + L);
    float distance = length(lights[i].position - f_in.position);
    float attenuation = 1.0 / (distance * distance);
    vec3 radiance = lights[i].colour * attenuation;

    // Cook-Torrance BRDF
    float D = distributionGGX(N, H, roughness);
    vec3 F = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);
    float G = geometrySmith(N, V, L, roughness);

    vec3 numerator = D * F * G;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;

    // light reflected (kS) is equal to Fresnel
    vec3 kS = F;
    // light refracted follows law of energy conservation
    vec3 kD = vec3(1.0) - kS;
    // multiply by inverse metalness such that only non-metals have diffuse lighting
    // since pure metals have no diffuse light
    kD *= 1.0 - metallic;

    // scale light by NdotL
    float NdotL = max(dot(N, L), 0.0);

    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
  }

  // ambient lighting
  float NdotV = max(dot(N, V), 0.0);
  vec3 F = fresnelSchlickRoughness(NdotV, F0, roughness);
  vec3 kS = F;
  vec3 kD = 1.0 - kS;
  kD *= 1.0 - metallic;

  // diffuse lighting
  vec3 irradiance = texture(irradianceMap, N).rgb;
  vec3 diffuse = irradiance * albedo;

  // specular lighting
  const float MAX_REFLECTION_LOD = 4.0;
  vec3 R = reflect(-V, N);
  vec3 prefilteredColour = textureLod(prefilterMap, R, roughness * MAX_REFLECTION_LOD).rgb;
  vec2 envBRDF = texture(brdfLUT, vec2(NdotV, roughness)).rg;
  vec3 specular = prefilteredColour * (F * envBRDF.x + envBRDF.y);
  // vec3 specular = prefilteredColour;

  vec3 ambient = (kS * diffuse + specular) * ambientOcclusion;

  vec3 colour = ambient + Lo;
  // HDR tone mapping using Reinhard operator
  colour = colour / (colour + vec3(1.0));
  // gamma correction
  colour = pow(colour, vec3(1.0 / 2.2));

  FragColor = vec4(colour, 1.0);
}
