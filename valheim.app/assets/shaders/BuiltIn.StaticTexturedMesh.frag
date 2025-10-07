#version 450

in vec2 FragTexCoords;
in vec3 FragNormals;
in vec3 FragColor;

out vec4 Color;

uniform sampler2D texture;
uniform bool UseTexture = true;

void main()
{
    if (UseTexture)
    {
        Color = texture(texture, FragTexCoords) * vec4(FragColor, 1.0);
    }
    else 
    {
        Color = vec4(FragColor, 1.0f);
    }
}
