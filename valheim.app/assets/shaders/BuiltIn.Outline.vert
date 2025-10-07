#version 450

layout(location = 0) in vec3 InPosition;

layout(binding = 0) uniform PerFrameData
{
	mat4 projection;
	mat4 view;
	mat4 model;
};

uniform float OutlineThickness;

void main()
{
	vec3 Expanded = InPosition * OutlineThickness;
	gl_Position = projection * view * model * vec4(Expanded, 1.0f);
}
