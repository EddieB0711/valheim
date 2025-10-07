#version 450

layout(location = 0) in vec3 InPos;
layout(location = 1) in vec3 InNormals;
layout(location = 2) in vec3 InColor;

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
    FragNormals = InNormals;
    FragColor = InColor;

    gl_Position = projection * view * model * vec4(InPos, 1.0f);
}
