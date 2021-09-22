#shader vertex
#version 450 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;

struct VertexOutput
{
    vec4 Color;
};

layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};


layout(location = 0) out VertexOutput Output;

void main()
{
    gl_Position = u_ViewProjection * a_Position;
    Output.Color = a_Color;
}

#shader fragment
#version 450 core

struct VertexOutput
{
    vec4 Color;
};

layout(location = 0) in VertexOutput Input;

layout(location = 0) out vec4 color;
layout(location = 1) out int entityId;

void main()
{
    color = Input.Color;
    entityId = -1;
}