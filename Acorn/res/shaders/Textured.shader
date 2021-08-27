#shader vertex
#version 330 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec2 a_TexCoord;

out vec2 v_TexCoord;

uniform mat4 u_ViewProjection;
uniform mat4 u_Transform;

void main()
{
	gl_Position = u_ViewProjection * u_Transform * a_Position;
	v_TexCoord = a_TexCoord;
}

#shader fragment
#version 330 core

in vec2 v_TexCoord;

uniform sampler2D u_Texture;

out vec4 color;

void main()
{
	color = texture(u_Texture, v_TexCoord);
}