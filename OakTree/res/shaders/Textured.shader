#shader vertex
#version 450 core

layout(location = 0) in vec4 a_Position;
layout(location = 1) in vec4 a_Color;
layout(location = 2) in vec2 a_TexCoord;
layout(location = 3) in float a_TexIndex;
layout(location = 4) in float a_TilingFactor;
layout(location = 5) in int a_EntityId;

// out vec2 v_TexCoord;
// out vec4 v_Color;
// out float v_TexIndex;
// out float v_TilingFactor;
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
};

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

layout(location = 0) out VertexOutput Output;
layout(location = 3) flat out float v_TexIndex;
layout(location = 4) flat out int v_EntityId;

// uniform mat4 u_ViewProjection;

void main()
{
	gl_Position = u_ViewProjection * a_Position;
	
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TilingFactor = a_TilingFactor;
	v_TexIndex = a_TexIndex;
	v_EntityId = a_EntityId;
}

#shader fragment
#version 450 core

struct VertexOutput
{
	vec4 Color;
	vec2 TexCoord;
	float TilingFactor;
};

layout(location = 0) in VertexOutput Input;
layout(location = 3) flat in float v_TexIndex;
layout(location = 4) in flat int v_EntityId;

layout(location = 0) out vec4 color;
layout(location = 1) out int entityId;

// in vec2 v_TexCoord;
// in vec4 v_Color;
// in float v_TexIndex;
// in float v_TilingFactor;
// flat in int v_EntityId;


layout (binding = 0) uniform sampler2D u_Textures[32];

void main()
{
	color = texture(u_Textures[int(v_TexIndex)], Input.TexCoord * Input.TilingFactor) * Input.Color;
	entityId = v_EntityId;
}