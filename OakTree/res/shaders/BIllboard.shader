#shader vertex
#version 450 core

layout(location = 0) in vec3 a_SquareVertices;
layout(location = 1) in vec3 a_BillboardPosition;
layout(location = 2) in vec2 a_BillboardScale;
layout(location = 3) in vec4 a_Color;
layout(location = 4) in vec2 a_TexCoord;
layout(location = 5) in float a_TexIndex;
layout(location = 6) in int a_EntityId;

struct VertexOutput
{
	vec2 TexCoord;
	vec4 Color;
	float TexIndex;
};

layout(location = 0) out VertexOutput Output;
layout(location = 4) out flat int v_EntityId;

// out vec2 v_TexCoord;
// out vec4 v_Color;
// out float v_TexIndex;
// flat out int v_EntityId;

// uniform mat4 u_ViewProjection;
// uniform vec3 u_CameraRight;
// uniform vec3 u_CameraUp;
layout(std140, binding = 0) uniform Camera
{
	mat4 u_ViewProjection;
	vec3 u_CameraRight;
	vec3 u_CameraUp;
};

void main()
{
	Output.Color = a_Color;
	Output.TexCoord = a_TexCoord;
	Output.TexIndex = a_TexIndex;
	v_EntityId = a_EntityId;

	vec3 centerWorldspace = a_BillboardPosition;

	vec3 vertexPositionWorldspace = 
		centerWorldspace
		+ u_CameraRight * a_SquareVertices.x * a_BillboardScale.x
		+ u_CameraUp * a_SquareVertices.y * a_BillboardScale.y;
	
	gl_Position = u_ViewProjection * vec4(vertexPositionWorldspace, 1.0);
}

#shader fragment
#version 450 core

struct VertexOutput
{
	vec2 TexCoord;
	vec4 Color;
	float TexIndex;
};

layout(location = 0) in VertexOutput Input;
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
	color = texture(u_Textures[int(Input.TexIndex)], Input.TexCoord) * Input.Color;
	entityId = v_EntityId;
}