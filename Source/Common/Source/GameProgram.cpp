#include <GameProgram.hpp>
#include <GameWindow.hpp>
#include <GitVersion.hpp>
#include <FileSystem.hpp>
#include <iostream>
#include <sstream>

namespace Brawl
{
	GameProgram::GameProgram( ) :
		m_pGameWindow( nullptr )
	{
	}

	GameProgram::~GameProgram( )
	{
		if( m_pGameWindow )
		{
			delete m_pGameWindow;
		}
	}

	ErrorCode GameProgram::Initialise( )
	{
		std::cout << "Game initialising" << std::endl;

		if( this->PlatformInitialise( ) != ErrorCode::Okay )
		{
			std::cout << "Failed to perform platform initialisation" <<
				std::endl;

			return ErrorCode::FatalError;
		}

		if( FileSystem::Get( ).Initialise( ) != ErrorCode::Okay )
		{
			std::cout << "Failed to initialise the file system" << std::endl;

			return ErrorCode::FatalError;
		}

		GameWindowParameters WindowParameters;

		WindowParameters.X = 0;
		WindowParameters.Y = 0;
		WindowParameters.Width = 800;
		WindowParameters.Height = 600;
		WindowParameters.Fullscreen = False;

		std::stringstream WindowName;
		WindowName << "Brawl";

#if defined( BRAWL_BUILD_DEBUG )
		WindowName << " [Debug] | Version: " << GIT_BUILD_VERSION << 
			" | Hash: " << GIT_COMMITHASH <<
			" | Tag: " << GIT_TAG_NAME <<
			" | Branch: " << GIT_BRANCH;
#endif // BRAWL_BUILD_DEBUG

		WindowParameters.Name = WindowName.str( );

		if( m_pGameWindow->Create( WindowParameters ) != ErrorCode::Okay )
		{
			std::cout << "[Brawl::GameProgram] <ERROR> "
				"Failed to create the window" << std::endl;

			return ErrorCode::FatalError;
		}

		if( m_Renderer.Initialise( m_pGameWindow ) != ErrorCode::Okay )
		{
			std::cout << "[Brawl::GameProgram] <ERROR> "
				"Failed to initialise the renderer" << std::endl;

			return ErrorCode::FatalError;
		}

		m_pGameWindow->SetRenderer( &m_Renderer );

		return ErrorCode::Okay;
	}

	ErrorCode GameProgram::Execute( )
	{
		std::cout << "Game executing" << std::endl;

		while( m_pGameWindow->IsOpen( ) )
		{
			m_pGameWindow->ProcessEvents( );
			m_Renderer.DrawFrame( );
		}

		m_pGameWindow->Destroy( );

		return ErrorCode::Okay;
	}
}

