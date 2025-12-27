#version 330 core

in vec2 texCoords;

uniform sampler2D positionBuffer;
uniform sampler2D normalBuffer;
uniform sampler2D noiseBuffer;

out float FragColor;

uniform vec3 samples[64];
uniform mat4 projection;

const float KERNEL_SIZE = 64;
const float RADIUS = 0.5;
const float BIAS = 0.025;

// tile noise texture over screen, based on screen dimensions / noise size
const vec2 noiseScale = vec2(800.0/4.0, 600.0/4.0);

void main() {
  // Sample provided buffers
  vec3 fragPos = texture(positionBuffer, texCoords).xyz;
  vec3 normal = texture(normalBuffer, texCoords).xyz;
  vec3 randomVec = texture(noiseBuffer, texCoords * noiseScale).xyz;

  // Tangent-Space
  vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
  vec3 bitangent = cross(normal, tangent);
  mat3 TBN = mat3(tangent, bitangent, normal);

  float occlusion = 0.0;
  for (int i=0; i<KERNEL_SIZE; ++i) {
    // 1. Manipulate the kernel
    vec3 samplePos = TBN * samples[i]; // from tangent to view-space
    samplePos = fragPos + samplePos * RADIUS;

    // 2. Normalise the offset
    vec4 offset = vec4(samplePos, 1.0);
    offset = projection * offset;         // from view to clip-space
    offset.xyz /= offset.w;               // perspective divide
    offset.xyz = offset.xyz * 0.5 + 0.5;  // transform to range 0.0 to 1.0

    // 3. Get the depth of the randomly picked fragment and perform a range check
    float sampleDepth = texture(positionBuffer, offset.xy).z;
    float rangeCheck = smoothstep(0.0, 1.0, RADIUS / abs(fragPos.z - sampleDepth));
    occlusion += (sampleDepth >= samplePos.z + BIAS ? 1.0 : 0.0) * rangeCheck;
  }

  occlusion = 1.0 - (occlusion / KERNEL_SIZE);
  FragColor = occlusion;
}
