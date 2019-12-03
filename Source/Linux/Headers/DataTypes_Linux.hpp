#ifndef __BRAWL_DATATYPES_HPP__
#error This file should not be included directly (DataTypes_Linux.hpp).  Include DataTypes.hpp, instead.
#endif // __BRAWL_DATATYPES_HPP__

#ifndef __BRAWL_DATATYPES_LINUX_HPP__
#define __BRAWL_DATATYPES_LINUX_HPP__

#include <cstdio>

namespace Brawl
{
	typedef std::FILE	*FileHandle;
	typedef int			FileDescriptor;
	typedef void		*LibraryHandle;

	extern const decltype( Null ) InvalidFileHandle;
	extern const int InvalidFileDescriptor;
	extern const decltype( Null ) InvalidLibraryHandle;
}

#endif // __BRAWL_DATATYPES_LINUX_HPP__

