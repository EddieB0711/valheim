#version 450

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec2 InTexCoords;
layout(location = 2) in vec3 InNormals;
layout(location = 3) in vec3 InColor;

out vec2 FragTexCoords;
out vec3 FragNormals;
out vec3 FragColor;

layout(binding = 0) uniform PerFrameData
{
    mat4 projection;
    mat4 view;
    mat4 model;
};

void main()
{
    FragTexCoords = InTexCoords;
    FragNormals = mat3(transpose(inverse(model))) * InNormals;
    FragColor = InColor;

    gl_Position = projection * view * model * vec4(InPosition, 1.0f);
}
