#version 450

layout(location = 0) in vec3 InPosition;
layout(location = 1) in vec2 InTexCoords;
layout(location = 1) in vec3 InNormal;

out vec3 FragPosition;
out vec3 FragNormal;
out vec2 FragTexCoords;

layout(binding = 0) uniform PerFrameData
{
    mat4 projection;
    mat4 view;
    mat4 model;
};

void main()
{
    FragPosition = vec3(model * vec4(InPosition, 1.0));
    FragNormal = mat3(transpose(inverse(model))) * InNormal;
    FragTexCoords = InTexCoords;

    gl_Position = projection * view * vec4(FragPosition, 1.0);
}

