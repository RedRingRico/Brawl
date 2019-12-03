#ifndef __BRAWL_GAMEWINDOWXCB_HPP__
#define __BRAWL_GAMEWINDOWXCB_HPP__

#include <GameWindow.hpp>
#include <xcb/xcb.h>

namespace Brawl
{
	class Renderer;

	class GameWindowDataXCB : public GameWindowData
	{
	public:
		GameWindowDataXCB( xcb_connection_t *p_pConnection,
			xcb_window_t p_Window );
		~GameWindowDataXCB( ) override;

		xcb_connection_t *GetConnection( ) const;
		xcb_window_t GetWindow( ) const;

	private:
		xcb_connection_t	*m_pConnection;
		xcb_window_t		m_Window;
	};

	class GameWindowXCB : public GameWindow
	{
	public:
		GameWindowXCB( );
		~GameWindowXCB( );

		// No copy, assignment, or move allowed
		GameWindowXCB( const GameWindowXCB & ) = delete;
		GameWindowXCB( const GameWindowXCB && ) = delete;
		GameWindowXCB &operator=( const GameWindowXCB & ) = delete;
		GameWindowXCB &operator=( const GameWindowXCB && ) = delete;

		ErrorCode Create(
			const GameWindowParameters &p_WindowParameters ) override;
		void Destroy( ) override;

		void ProcessEvents( ) override;

	private:
		xcb_connection_t		*m_pConnection;
		xcb_window_t			m_Window;
		xcb_intern_atom_reply_t	*m_pDeleteReply;
	};
}

#endif // __BRAWL_GAMEWINDOWXCB_HPP__

