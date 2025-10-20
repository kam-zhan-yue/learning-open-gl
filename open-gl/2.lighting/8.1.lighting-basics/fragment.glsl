#version 330 core

out vec4 FragColor;

uniform vec3 lightColour;
uniform vec3 objectColour;

void main()
{
    FragColor = vec4(lightColour * objectColour, 1.0);
}
