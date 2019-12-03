#ifndef __BRAWL_GAMEPROGRAM_HPP__
#define __BRAWL_GAMEPROGRAM_HPP__

#include <DataTypes.hpp>
#include <Renderer.hpp>

namespace Brawl
{
	class GameWindow;

	class GameProgram
	{
	public:
		GameProgram( );
		~GameProgram( );

		ErrorCode Initialise( );
		ErrorCode Execute( );

	private:
		ErrorCode PlatformInitialise( );

		GameWindow	*m_pGameWindow;
		Renderer	m_Renderer;
	};
}

#endif // __BRAWL_GAMEPROGRAM_HPP__

