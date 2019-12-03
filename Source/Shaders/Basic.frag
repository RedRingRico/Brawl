#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input
layout( location = 0 ) in vec3 i_Colour;

// Output
layout( location = 0 ) out vec4 o_Colour;

void main( )
{
	o_Colour = vec4( i_Colour, 1.0 );
}
