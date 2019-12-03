#version 450
#extension GL_ARB_separate_shader_objects : enable

// Output
layout( location = 0 ) out vec3 o_Colour;

// Locals
vec2 Positions[ 3 ] = vec2[ ](
	vec2( 0.0, -0.5 ),
	vec2( 0.5, 0.5 ),
	vec2( -0.5, 0.5 )
);

vec3 Colours[ 3 ] = vec3[ ](
	vec3( 1.0, 0.0, 0.0 ),
	vec3( 0.0, 1.0, 0.0 ),
	vec3( 0.0, 0.0, 1.0 )
);

void main( )
{
	gl_Position = vec4( Positions[ gl_VertexIndex ], 0.0, 1.0 );
	o_Colour = Colours[ gl_VertexIndex ];
}
