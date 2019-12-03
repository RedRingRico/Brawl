#ifndef __BRAWL_DATATYPES_HPP__
#define __BRAWL_DATATYPES_HPP__

#if defined ( __GNUC__ )
#include <Compiler_GCC.hpp>
#endif // __GNUC__

// Compiler-agnostic data types
namespace Brawl
{
	typedef float	Float32;
	typedef double	Float64;

	typedef UInt8	Byte;
	typedef MemSize	Bool;

	const Bool True = 1;
	const Bool False = 0;
}

#include <ErrorCodes.hpp>

#if defined ( __linux__ )
#include <DataTypes_Linux.hpp>
#else
#error No platform detected
#endif // __linux__

#include <string>

namespace Brawl
{
	ErrorCode GetCompilerString( std::string &p_CompilerString );

	// Endian swapping
	BRAWL_INLINE UInt16 SwapEndianUInt16( UInt16 p_Swap )
	{
		return	( ( p_Swap & 0x00FF ) << 8 ) |
				( ( p_Swap & 0xFF00 ) >> 8 );
	}

	BRAWL_INLINE UInt32 SwapEndianUInt32( UInt32 p_Swap )
	{
		return	( ( p_Swap & 0x000000FF << 24 ) |
				( ( p_Swap & 0x0000ff00 ) << 8 ) |
				( ( p_Swap & 0x00FF0000 ) >> 8 ) |
				( ( p_Swap & 0xFF000000 ) >> 24 ) );
	}

#define Align( p_Memory, p_Size ) \
	( ( ( MemSize )( p_Memory ) + ( p_Size ) - 1 ) & \
	( ~( ( p_Size ) - 1 ) ) )
#define AlignUp( p_Memory, p_Size ) ( Align( ( p_Memory ), p_Size ) + \
	( p_Size ) )

#if defined ( BRAWL_LITTLE_ENDIAN )
#define EndianToBigUInt16( p_Swap ) SwapEndianUInt16( p_Swap )
#define EndianToLittleUInt16( p_Swap )
#define EndianToBigUInt32( p_Swap ) SwapEndianUInt32( p_Swap )
#define EndianToLittleUInt32( p_Swap )
#elif defined ( BRAWL_BIG_ENDIAN )
#define EndianToBigUInt16( p_Swap )
#define EndianToLittleUInt16( p_Swap ) SwapEndianUInt16( p_Swap )
#define EndianToBigUInt32( p_Swap )
#define EndianToLittleUInt32( p_Swap ) SwapEndianUInt32( p_Swap )
#else
#error No endianness defined
#endif // BRAWL_LITTLE_ENDIAN
}


#endif // __BRAWL_DATATYPES_HPP__

