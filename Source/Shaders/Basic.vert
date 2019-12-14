#version 450
#extension GL_ARB_separate_shader_objects : enable

// Input
layout( location = 0 ) in vec2 i_Position;
layout( location = 1 ) in vec3 i_Colour;

// Output
layout( location = 0 ) out vec3 o_Colour;

void main( )
{
	gl_Position = vec4( i_Position, 0.0, 1.0 );
	o_Colour = i_Colour;
}
