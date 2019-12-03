#include <GameProgram.hpp>
#include <GameWindowXCB.hpp>
#include <iostream>

namespace Brawl
{
	ErrorCode GameProgram::PlatformInitialise( )
	{
		m_pGameWindow = new GameWindowXCB( );

		if( m_pGameWindow == nullptr )
		{
			std::cout << "Failed to create the game window" << std::endl;

			return ErrorCode::WindowCreationFailed;
		}

		return ErrorCode::Okay;
	}
}

