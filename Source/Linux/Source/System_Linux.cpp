#include <System.hpp>
#include <iostream>
#include <dlfcn.h>

namespace Brawl
{
	LibraryHandle OpenLibrary( const std::string &p_LibraryName,
		const LibraryFlags p_Flags )
	{
		LibraryHandle OpenedLibrary = InvalidLibraryHandle;
		int OpenFlags = 0;

		if( p_Flags & LibraryFlags::LazyBind )
		{
			OpenFlags = RTLD_LAZY;
		}

		// Overrides LazyBind, if specified
		if( p_Flags & LibraryFlags::ImmediateBind )
		{
			OpenFlags = RTLD_NOW;
		}

		// Can not allow this to be zero!
		if( OpenFlags == 0 )
		{
			// Check to see if LD_BIND_NOW is in the environment
			if( getenv( "LD_BIND_NOW" ) == nullptr )
			{
				std::cout << "Lazy/Immediate bind not specified" << std::endl;
				goto OpenLibraryEnd;
			}

			std::cout << "Using the LD_BIND_NOW environment variable for "
				"library binding" << std::endl;
		}

		// Additionally, OR these in
		if( p_Flags & LibraryFlags::GlobalSymbols )
		{
			OpenFlags |= RTLD_GLOBAL;
		}

		if( p_Flags & LibraryFlags::LocalSymbols )
		{
			OpenFlags |= RTLD_LOCAL;
		}

		// glibc 2.2+
		if( p_Flags & LibraryFlags::LeaveOpen )
		{
			OpenFlags |= RTLD_NODELETE;
		}

		// glibc 2.2+
		if( p_Flags & LibraryFlags::DontLoad )
		{
			OpenFlags |= RTLD_NOLOAD;
		}

		// glibc 2.3.4+
		if( p_Flags & LibraryFlags::DeepBind )
		{
			OpenFlags |= RTLD_DEEPBIND;
		}

		if( ( OpenedLibrary = dlopen( p_LibraryName.c_str( ), OpenFlags ) )
			== InvalidLibraryHandle )
		{
			std::cout << "Failed to Open library: " << p_LibraryName <<
				std::endl;
		}

		std::cout << "Opened library: " << p_LibraryName << std::endl;

OpenLibraryEnd:

		return OpenedLibrary;
	}

	ErrorCode CloseLibrary( LibraryHandle p_Handle )
	{
		int CloseResult = dlclose( p_Handle );
		switch( CloseResult )
		{
			case 0:
			{
				break;
			}
			default:
			{
				std::cout << "Unknown error: " << CloseResult << std::endl;
				return ErrorCode::CloseLibraryFailed;
			}
		}

		return ErrorCode::Okay;
	}
}

