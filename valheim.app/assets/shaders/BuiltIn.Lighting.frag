#version 450

in vec3 FragNormal;
in vec3 FragPosition;

out vec4 FragColor;

uniform vec3 LightPosition;
uniform vec3 ViewPosition;
uniform vec3 LightColor;
uniform vec3 ObjectColor;
uniform bool UseBlinn;

void main()
{
    float AmbientStrength = 0.05;
    vec3 Ambient = AmbientStrength * LightColor;
    vec3 Norm = normalize(FragNormal);
    vec3 LightDir = normalize(LightPosition - FragPosition);

    float Diff = max(dot(LightDir, Norm), 0.0);
    vec3 Diffuse = Diff * LightColor;

    float SpecularStrength = 0.5;
    vec3 ViewDir = normalize(ViewPosition - FragPosition);

    if (UseBlinn)
    {
        vec3 HalfwayDir = normalize(LightDir + ViewDir);
		float Spec = pow(max(dot(ViewDir, HalfwayDir), 0.0), 32);
		vec3 Specular = SpecularStrength * Spec * LightColor;

		vec3 result = (Ambient + Diffuse + Specular);
		FragColor = vec4(result, 1.0);
    }
    else
    {
        vec3 ReflectDir = reflect(-LightDir, Norm);
		float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), 8);
		vec3 Specular = SpecularStrength * Spec * LightColor;

		vec3 result = (Ambient + Diffuse + Specular);
		FragColor = vec4(result, 1.0);
    }
}
