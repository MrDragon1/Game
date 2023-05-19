#version 460 core
layout(location = 0) out vec4 FragColor;
layout(location = 1) out int EntityID;

layout(location = 0) in vec2 TexCoords;

layout(binding = 0) uniform sampler2D texture1;

void main()
{    
    FragColor = texture(texture1, TexCoords);
    EntityID = 122;
}