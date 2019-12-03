#include <GameWindow.hpp>

namespace Brawl
{
	GameWindowData::~GameWindowData( )
	{
	}

	GameWindowType GameWindowData::GetType( ) const
	{
		return m_Type;
	}

	GameWindow::~GameWindow( )
	{
	}

	GameWindowData *GameWindow::GetGameWindowData( )
	{
		return m_pGameWindowData;
	}

	void GameWindow::SetRenderer( Renderer *p_pRenderer )
	{
	    m_pRenderer = p_pRenderer;
	}

	Bool GameWindow::IsOpen( ) const
	{
		return m_Open;
	}

	UInt32 GameWindow::GetWidth( ) const
	{
		return m_Width;
	}

	UInt32 GameWindow::GetHeight( ) const
	{
		return m_Height;
	}
}

