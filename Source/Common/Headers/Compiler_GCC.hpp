#ifndef __BRAWL_DATATYPES_HPP__
#error This file should not be included directly (Compiler_GCC.hpp).  Include DataTypes.hpp, instead.
#endif // __BRAWL_DATATYPES_HPP__

#ifndef __BRAWL_COMPILER_GCC_HPP__
#define __BRAWL_COMPILER_GCC_HPP__

#if __cplusplus <= 199711L
#error Unsupported compiler (C++11 required)
#else
#include <cstddef>
#include <cstdint>
#include <endian.h>

namespace Brawl
{
	typedef signed char		SChar8;
	typedef unsigned char	UChar8;
	typedef std::int8_t		SInt8;
	typedef std::uint8_t	UInt8;
	typedef std::int16_t	SInt16;
	typedef std::uint16_t	UInt16;
	typedef std::int32_t	SInt32;
	typedef std::uint32_t	UInt32;
	typedef std::int64_t	SInt64;
	typedef std::uint64_t	UInt64;
	typedef std::size_t		MemSize;

#define Null nullptr
}

#define BRAWL_COMPILER_VERSION ( __GNUc__ * 10000 + __GNUC_MINOR__ * 100 + \
	__GNUC_PATCHLEVEL__ )

#define BRAWL_COMPILER_GCC

#define BRAWL_PACKDATA_BEGIN( p_Align )
#define BRAWL_PACKDATA_END( p_Align ) __attribute__( ( __aligned( p_Align ) ) )

#define BRAWL_INLINE __inline__
#define BRAWL_FORCE_INLINE __forceinline__

#define BRAWL_EXPLICIT explicit
#define BRAWL_ASM __asm__

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define BRAWL_LITTLE_ENDIAN
#else
#define BRAWL_BIG_ENDIAN
#endif // __BYTE_ORDER

#endif // __cplusplus <= 199711L

#endif // __BRAWL_COMPILER_GCC_HPP__

