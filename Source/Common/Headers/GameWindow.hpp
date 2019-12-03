#ifndef __BRAWL_GAMEWINDOW_HPP__
#define __BRAWL_GAMEWINDOW_HPP__

#include <DataTypes.hpp>

namespace Brawl
{
	// Determines the position, dimensions, and if the window is fullscreen
	struct GameWindowParameters
	{
		std::string	Name;
		SInt32		X;
		SInt32		Y;
		UInt32		Width;
		UInt32		Height;
		Bool		Fullscreen;
	};

	// XCB, Win32 window
	enum class GameWindowType : Byte
	{
		XCB = 0,
		Unknown
	};

	class Renderer;

	// Holds windowing system specific data
	class GameWindowData
	{
	public:
		virtual ~GameWindowData( );

		GameWindowType GetType( ) const;

	protected:
		GameWindowType	m_Type;
	};

	class GameWindow
	{
	public:
		virtual ~GameWindow( );

		virtual ErrorCode Create(
			const GameWindowParameters &p_WindowParameters ) = 0;
		virtual void Destroy( ) = 0;

		virtual void ProcessEvents( ) = 0;

		GameWindowData *GetGameWindowData( );

		void SetRenderer( Renderer *p_pRenderer );

		Bool IsOpen( ) const;

		UInt32 GetWidth( ) const;
		UInt32 GetHeight( ) const;

	protected:
		GameWindowData	*m_pGameWindowData;
		Renderer		*m_pRenderer;

		SInt32	m_X;
		SInt32	m_Y;
		UInt32	m_Width;
		UInt32	m_Height;

		Bool	m_Open;
		Bool	m_Resize;
	};
}

#endif // __BRAWL_GAMEWINDOW_HPP__

