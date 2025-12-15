#version 330 core

out vec4 FragColor;

in vec3 ourColour;
in vec4 vertexPosition;
in vec2 texCoords;

uniform sampler2D ourTexture;

void main()
{
    FragColor = texture(ourTexture, texCoords) * vec4(ourColour, 1.0f);
}
