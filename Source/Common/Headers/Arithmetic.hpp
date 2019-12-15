#ifndef __BRAWL_ARITHMETIC_HPP__
#define __BRAWL_ARITHMETIC_HPP__

#include <DataTypes.hpp>

namespace Brawl
{
	const Float32 kEpsilon = 1.0e-10;
	const Float32 kHalfEpsilon = 1.0e-5;
	const Float32 kPi = 3.1415926535897932384626433832795f;

	Bool IsZero( Float32 p_Value );
	Bool IsEqual( Float32 p_Left, Float32 p_Right );

	Float32 SquareRoot( Float32 p_Value );
	Float32 InverseSquareRoot( Float32 p_Value );
	Float32 Absolute( Float32 p_Value );

	void SineCosine( Float32 p_Angle, Float32 &p_Sine, Float32 &p_Cosine );
}

#endif // __BRAWL_ARITHMETIC_HPP__
