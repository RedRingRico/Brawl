#include <Arithmetic.hpp>
#include <cmath>

namespace Brawl
{
	Bool IsZero( Float32 p_Value )
	{
		return ( fabs( p_Value ) < kEpsilon );
	}

	Bool IsEqual( Float32 p_Left, Float32 p_Right )
	{
		return ( IsZero( p_Left - p_Right ) );
	}

	Float32 SquareRoot( Float32 p_Value )
	{
		return sqrtf( p_Value );
	}

	Float32 InverseSquareRoot( Float32 p_Value )
	{
		return ( 1.0f / SquareRoot( p_Value ) );
	}

	Float32 Absolute( Float32 p_Value )
	{
		return fabs( p_Value );
	}

	void SineCosine( Float32 p_Angle, Float32 &p_Sine, Float32 &p_Cosine )
	{
		p_Sine = sinf( p_Angle );
		p_Cosine = cosf( p_Angle );
	}
}
