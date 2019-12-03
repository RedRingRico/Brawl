#ifndef __BRAWL_SYSTEM_HPP__
#define __BRAWL_SYSTEM_HPP__

#include <DataTypes.hpp>

namespace Brawl
{
	enum class LibraryFlags : SInt32
	{
		None = 0,
		LazyBind = 1,
		ImmediateBind = 2,
		GlobalSymbols = 4,
		LocalSymbols = 8,
		LeaveOpen = 16,
		DontLoad = 32,
		DeepBind = 64
	};

	LibraryFlags operator|( LibraryFlags p_Left, LibraryFlags p_Right );
	LibraryFlags operator|=( LibraryFlags p_Left, LibraryFlags p_Right );

	bool operator&( LibraryFlags p_Left, LibraryFlags p_Right );

	LibraryHandle OpenLibrary( const std::string &p_LibraryName,
		const LibraryFlags p_Flags );

	ErrorCode CloseLibrary( LibraryHandle p_Handle );
#if defined ( BRAWL_PLATFORM_LINUX )
#include <dlfcn.h>
#define LoadProcAddress dlsym
#else
#error Unknown platform
#endif // BRAWL_PLATFORM_LINUX
}

#endif // __BRAWL_SYSTEM_HPP__

