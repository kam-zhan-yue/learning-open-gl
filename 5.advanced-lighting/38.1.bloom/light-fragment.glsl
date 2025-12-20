#version 330 core

layout(location = 0) out vec4 FragColor;
layout(location = 1) out vec4 BrightColor;

uniform vec3 lightColour;

void main() {
  FragColor = vec4(lightColour, 1.0);
  float brightness = dot(FragColor.rgb, vec3(0.2126, 0.7152, 0.0722));
  BrightColor = brightness > 1.0 ? vec4(FragColor.rgb, 1.0) : vec4(vec3(0.0), 1.0);

  // FragColor = BrightColor;
  // BrightColor = vec4(lightColour, 1.0);
}
