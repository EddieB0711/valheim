#version 450

in vec3 FragPosition;
in vec2 FragTexCoords;
in vec3 FragNormal;

out vec4 FragColor;

uniform vec3 LightPosition;
uniform vec3 ViewPosition;
uniform vec3 LightColor;
uniform vec3 ObjectColor;

uniform sampler2D texture;

void main()
{
    vec4 TextureColor = texture(texture, FragTexCoords);

    float AmbientStrength = 0.1;
    vec3 Ambient = AmbientStrength * TextureColor.rgb;
    vec3 Norm = normalize(FragNormal);
    vec3 LightDir = normalize(LightPosition - FragPosition);

    float Diff = max(dot(Norm, LightDir), 0.0);
    vec3 Diffuse = Diff * LightColor * TextureColor.rgb;

    float SpecularStrength = 0.5;
    vec3 ViewDir = normalize(ViewPosition - FragPosition);
    vec3 ReflectDir = reflect(-LightDir, Norm);

    float Spec = pow(max(dot(ViewDir, ReflectDir), 0.0), 32);
    vec3 Specular = SpecularStrength * Spec * LightColor * TextureColor.rgb;

    vec3 result = (Ambient + Diffuse + Specular) * ObjectColor;

    FragColor = vec4(result, TextureColor.a);
}
