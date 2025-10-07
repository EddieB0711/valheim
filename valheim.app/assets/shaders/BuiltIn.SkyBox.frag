#version 450

in vec3 FragTexCoords;

out vec4 FragColor;

uniform samplerCube SkyBox;

void main()
{
    FragColor = texture(SkyBox, FragTexCoords);
}