#version 450

uniform vec4 OutlineColor;

out vec4 FragColor;

void main()
{
	FragColor = OutlineColor;
}
