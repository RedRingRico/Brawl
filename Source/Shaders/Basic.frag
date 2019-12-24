#version 450
#extension GL_ARB_separate_shader_objects : enable

// Uniforms
layout( binding = 1 ) uniform sampler2D u_Sampler;

// Input
layout( location = 0 ) in vec3 i_Colour;
layout( location = 1 ) in vec2 i_UV;

// Output
layout( location = 0 ) out vec4 o_Colour;

void main( )
{
	o_Colour = texture( u_Sampler, i_UV );
}
