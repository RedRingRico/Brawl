#version 450
#extension GL_ARB_separate_shader_objects : enable

// Uniforms
layout( binding = 0 ) uniform UniformBufferObject
{
	mat4	Model;
	mat4	View;
	mat4	Projection;
}u_UBO;

// Input
layout( location = 0 ) in vec2 i_Position;
layout( location = 1 ) in vec3 i_Colour;
layout( location = 2 ) in vec2 i_UV;

// Output
layout( location = 0 ) out vec3 o_Colour;
layout( location = 1 ) out vec2 o_UV;

void main( )
{
	gl_Position = u_UBO.Projection * u_UBO.View * u_UBO.Model * vec4( i_Position, 0.0, 1.0 );
	//gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;
	o_Colour = i_Colour;
	o_UV = i_UV;
}
