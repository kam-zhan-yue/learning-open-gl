#version 330 core

in vec3 localPos;
out vec4 FragColor;

uniform samplerCube environmentMap;

const float PI = 3.14159265359;

void main() {
  // the sample direction equals the hemisphere's orientation
  vec3 normal = normalize(localPos);
  vec3 irradiance = vec3(0.0);

  vec3 up = vec3(0.0, 1.0, 0.0);
  vec3 right = cross(up, normal);
  up = cross(normal, right);

  float sampleDelta = 0.025;
  float nrSamples = 0.0;
  // increment the polar azimuth
  for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
    // increment the inclination angle
    for (float theta = 0.0; theta < 0.5; theta += sampleDelta) {
      // spherical to cartesian (in tangent space)
      vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
      // tangent space to world
      vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * normal;
      irradiance += texture(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);
      nrSamples++;
    }
  }
  irradiance = PI * irradiance * (1.0 / nrSamples);

  FragColor = vec4(irradiance, 1.0);
}
