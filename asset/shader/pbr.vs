#version 460 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in vec3 a_Tangent;
layout(location = 4) in vec3 a_Binormal;

uniform mat4 uViewProjection;
uniform mat4 uModel;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_TexCoord;

void main()
{
    v_TexCoord = a_TexCoord;
    v_WorldPos = vec3(uModel * vec4(a_Position, 1.0));
    v_Normal = mat3(uModel) * a_Normal;
    gl_Position = uViewProjection * vec4(v_WorldPos, 1.0);
}