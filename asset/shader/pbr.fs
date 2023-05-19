#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

in vec3 v_WorldPos;
in vec3 v_Normal;
in vec2 v_TexCoord;

uniform samplerCube uShadowMap;

void main()
{    
    FragColor = vec4(v_TexCoord,0.0,1.0);
    EntityID = 122;
}