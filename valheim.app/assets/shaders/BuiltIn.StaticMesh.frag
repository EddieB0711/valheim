#version 450

in vec3 FragNormals;
in vec3 FragColor;

out vec4 OutColor;

void main()
{
    OutColor = vec4(FragColor, 1.0);
}