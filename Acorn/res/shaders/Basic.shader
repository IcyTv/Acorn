#shader vertex
#version 330 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

out vec4 v_Color;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * a_Position;
	v_Color = a_Color;
}

#shader fragment
#version 330 core

in vec4 v_Color;

out vec4 color;

void main()
{
	color = v_Color;
}