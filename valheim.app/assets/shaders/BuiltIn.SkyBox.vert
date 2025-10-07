#version 450

layout(location = 0) in vec3 InPosition;

out vec3 FragTexCoords;

layout(std140) uniform PerFrameData
{
	mat4 projection;
	mat4 view;
	mat4 model;
};

void main()
{
	vec4 Pos = projection * mat4(mat3(view)) * vec4(InPosition, 1.0f);
	gl_Position = Pos.xyww;
	FragTexCoords = InPosition;
}