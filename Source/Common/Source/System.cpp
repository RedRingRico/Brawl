#include <System.hpp>

namespace Brawl
{
	LibraryFlags operator|( LibraryFlags p_Left, LibraryFlags p_Right )
	{
		SInt32 Return = static_cast< SInt32 >( p_Left );

		Return |= static_cast< SInt32 >( p_Right );
		
		return static_cast< LibraryFlags>( Return );
	}

	LibraryFlags operator|=( LibraryFlags p_Left, LibraryFlags p_Right )
	{
		LibraryFlags Return = p_Left;

		Return = Return | p_Right;

		return Return;
	}

	bool operator&( LibraryFlags p_Left, LibraryFlags p_Right )
	{
		return ( static_cast< SInt32 >( p_Left ) &
			static_cast< SInt32 >( p_Right ) );
	}
}

