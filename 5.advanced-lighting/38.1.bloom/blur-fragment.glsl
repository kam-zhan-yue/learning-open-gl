#version 330 core

out vec4 FragColor;
in vec2 texCoords;

uniform sampler2D image;
uniform bool horizontal;
// weights get less important the further it goes
uniform float weights[5] = float[] (0.2270270270, 0.1945945946, 0.1216216216, 0.0540540541, 0.0162162162);

void main() {
  // size of a single texel
  vec2 texOffset = 1.0 / textureSize(image, 0);
  // this fragment
  vec3 result = texture(image, texCoords).rgb * weights[0];
  if (horizontal) {
    for (int i=0; i<5; ++i) {
      result += texture(image, texCoords + vec2(texOffset.x * i, 0.0)).rgb * weights[i];
      result += texture(image, texCoords - vec2(texOffset.x * i, 0.0)).rgb * weights[i];
    }
  } else {
    for (int i=0; i<5; ++i) {
      result += texture(image, texCoords + vec2(texOffset.y * i, 0.0)).rgb * weights[i];
      result += texture(image, texCoords - vec2(texOffset.y * i, 0.0)).rgb * weights[i];
    }
  }
  FragColor = vec4(result, 1.0);
}
