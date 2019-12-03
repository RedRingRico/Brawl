#include <GameWindowXCB.hpp>
#include <iostream>
#include <xcb/xproto.h>
#include <xcb/randr.h>
#include <xcb/xfixes.h>
#include <Renderer.hpp>

namespace Brawl
{
	GameWindowDataXCB::GameWindowDataXCB( xcb_connection_t *p_pConnection,
		xcb_window_t p_Window ) :
		m_pConnection( p_pConnection ),
		m_Window( p_Window )
	{
		m_Type = GameWindowType::XCB;
	}

	GameWindowDataXCB::~GameWindowDataXCB( )
	{
	}

	xcb_connection_t *GameWindowDataXCB::GetConnection( ) const
	{
		return m_pConnection;
	}

	xcb_window_t GameWindowDataXCB::GetWindow( ) const
	{
		return m_Window;
	}

	GameWindowXCB::GameWindowXCB( ) :
		m_pConnection( nullptr ),
		m_Window( 0 ),
		m_pDeleteReply( nullptr )
	{
		m_X = 0;
		m_Y = 0;
		m_Width = 640;
		m_Height = 480;
		m_Open = False;
		m_pGameWindowData = nullptr;
		m_Resize = False;
	}

	GameWindowXCB::~GameWindowXCB( )
	{
		this->Destroy( );
	}

	ErrorCode GameWindowXCB::Create(
		const GameWindowParameters &p_WindowParameters )
	{
		// Just in case we've already created the window
		this->Destroy( );

		int ScreenIndex;

		m_pConnection = xcb_connect( nullptr, &ScreenIndex );

		int Error = xcb_connection_has_error( m_pConnection );

		if( Error != 0 )
		{
			std::cout << "Unable to open XCB display" << std::endl;
			std::cout << "ERROR: ";

			switch( Error )
			{
				case XCB_CONN_ERROR:
				{
					std::cout << "Stream";
					break;
				}
				case XCB_CONN_CLOSED_EXT_NOTSUPPORTED:
				{
					std::cout << "Unsupported extension";
					break;
				}
				case XCB_CONN_CLOSED_MEM_INSUFFICIENT:
				{
					std::cout << "Not enough memory";
					break;
				}
				case XCB_CONN_CLOSED_REQ_LEN_EXCEED:
				{
					std::cout << "Requested length exceeded";
					break;
				}
				case XCB_CONN_CLOSED_PARSE_ERR:
				{
					std::cout << "Failed to parse display string";
					break;
				}
				case XCB_CONN_CLOSED_INVALID_SCREEN:
				{
					std::cout << "No screen matching the display";
					break;
				}
				default:
				{
					std::cout << "UNKNOWN [" << Error << "]";
					break;
				}
			}

			std::cout << std::endl;

			return ErrorCode::XCBOpenDisplayFailed;
		}

		const xcb_setup_t *pSetup = xcb_get_setup( m_pConnection );
		xcb_screen_iterator_t ScreenIterator =
			xcb_setup_roots_iterator( pSetup );

		while( ScreenIndex-- > 0 )
		{
			xcb_screen_next( &ScreenIterator );
		}

		xcb_screen_t *pScreen = ScreenIterator.data;

		uint32_t WindowValues[ ] =
		{
			pScreen->black_pixel,
			XCB_EVENT_MASK_EXPOSURE | XCB_EVENT_MASK_KEY_PRESS |
				XCB_EVENT_MASK_STRUCTURE_NOTIFY
		};

		m_X = p_WindowParameters.X;
		m_Y = p_WindowParameters.Y;
		m_Width = p_WindowParameters.Width;
		m_Height = p_WindowParameters.Height;

		m_Window = xcb_generate_id( m_pConnection );

		// Create a window which will be destroyed to get a fullscreen one
		xcb_create_window( m_pConnection, XCB_COPY_FROM_PARENT, m_Window,
			pScreen->root, m_X, m_Y, m_Width, m_Height, 0,
			XCB_WINDOW_CLASS_INPUT_OUTPUT, pScreen->root_visual,
			XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, WindowValues );

		xcb_flush( m_pConnection );

		if( p_WindowParameters.Fullscreen == True )
		{
			// Use the primary display to set up the window
			xcb_generic_error_t *pError = nullptr;

			xcb_randr_get_output_primary_cookie_t RandrOutputPrimaryCookie =
				xcb_randr_get_output_primary( m_pConnection, m_Window );
			xcb_randr_get_output_primary_reply_t *pRandrOutputPrimaryReply =
				xcb_randr_get_output_primary_reply( m_pConnection,
					RandrOutputPrimaryCookie, &pError );

			if( pError != nullptr )
			{
				return ErrorCode::PrimaryOutputFailure;
			}

			xcb_randr_get_output_info_cookie_t RandrGetOutputInfoCookie =
				xcb_randr_get_output_info( m_pConnection,
					pRandrOutputPrimaryReply->output, 0 );
			xcb_randr_get_output_info_reply_t *pRandrGetOutputInfoReply =
				xcb_randr_get_output_info_reply( m_pConnection,
					RandrGetOutputInfoCookie, &pError );

			free( pRandrOutputPrimaryReply );

			if( pError != nullptr )
			{
				return ErrorCode::PrimaryOutputInfoFailure;
			}

			xcb_randr_get_crtc_info_cookie_t CRTCInfoCookie =
				xcb_randr_get_crtc_info( m_pConnection,
					pRandrGetOutputInfoReply->crtc, 0 );
			xcb_randr_get_crtc_info_reply_t *pCRTCInfoReply =
				xcb_randr_get_crtc_info_reply( m_pConnection, CRTCInfoCookie,
					0 );

			free( pRandrGetOutputInfoReply );

			std::cout << "[CRTC info]" << std::endl;
			std::cout << "\tX offset: " << pCRTCInfoReply->x << std::endl;
			std::cout << "\tY offset: " << pCRTCInfoReply->y << std::endl;
			std::cout << "\tWidth:    " << pCRTCInfoReply->width << std::endl;
			std::cout << "\tHeight:   " << pCRTCInfoReply->height << std::endl;

			m_X = pCRTCInfoReply->x;
			m_Y = pCRTCInfoReply->y;
			m_Width = pCRTCInfoReply->width;
			m_Height = pCRTCInfoReply->height;

			free( pCRTCInfoReply );

			xcb_destroy_window( m_pConnection, m_Window );

			// Recreate the window with the primary screen information
			xcb_create_window( m_pConnection, XCB_COPY_FROM_PARENT, m_Window,
				pScreen->root, m_X, m_Y, m_Width, m_Height, 0,
				XCB_WINDOW_CLASS_INPUT_OUTPUT, pScreen->root_visual,
				XCB_CW_BACK_PIXEL | XCB_CW_EVENT_MASK, WindowValues );

			xcb_intern_atom_cookie_t StateCookie = xcb_intern_atom(
				m_pConnection, 1, 13, "_NET_WM_STATE" );
			xcb_intern_atom_cookie_t FullscreenCookie = xcb_intern_atom(
				m_pConnection, 1, 24, "_NET_WM_STATE_FULLSCREEN" );
			xcb_intern_atom_reply_t *pStateReply = xcb_intern_atom_reply(
				m_pConnection, StateCookie, 0 );
			xcb_intern_atom_reply_t *pFullscreenReply = xcb_intern_atom_reply(
				m_pConnection, FullscreenCookie, 0 );
			xcb_change_property( m_pConnection, XCB_PROP_MODE_REPLACE,
				m_Window, ( *pStateReply ).atom, 4, 32, 1,
				&( *pFullscreenReply ).atom );

			free( pFullscreenReply );
			free( pStateReply );
		}

		xcb_map_window( m_pConnection, m_Window );
		xcb_flush( m_pConnection );

		xcb_xfixes_hide_cursor( m_pConnection, m_Window );

		m_Open = True;

		m_pGameWindowData = new GameWindowDataXCB( m_pConnection, m_Window );

		xcb_intern_atom_cookie_t ProtocolsCookie = xcb_intern_atom(
			m_pConnection, 1, 12, "WM_PROTOCOLS" );
		xcb_intern_atom_reply_t *pProtocolsReply = xcb_intern_atom_reply(
			m_pConnection, ProtocolsCookie, 0 );
		xcb_intern_atom_cookie_t DeleteCookie = xcb_intern_atom(
			m_pConnection, 0, 16, "WM_DELETE_WINDOW" );
		m_pDeleteReply = xcb_intern_atom_reply( m_pConnection, DeleteCookie,
			0 );

		xcb_change_property( m_pConnection, XCB_PROP_MODE_REPLACE, m_Window,
			( *pProtocolsReply ).atom, 4, 32, 1, &( *m_pDeleteReply ).atom );
		free( pProtocolsReply );

		if( p_WindowParameters.Name.size( ) > 0 )
		{
			xcb_change_property( m_pConnection, XCB_PROP_MODE_REPLACE,
				m_Window, XCB_ATOM_WM_NAME, XCB_ATOM_STRING, 8,
				p_WindowParameters.Name.size( ),
				p_WindowParameters.Name.c_str( ) );
		}

		xcb_flush( m_pConnection );

		return ErrorCode::Okay;
	}

	void GameWindowXCB::Destroy( )
	{
		if( m_pDeleteReply )
		{
			free( m_pDeleteReply );
			m_pDeleteReply = nullptr;
		}

		if( m_pGameWindowData )
		{
			delete m_pGameWindowData;
			m_pGameWindowData = nullptr;
		}

		if( m_pConnection )
		{
			xcb_xfixes_show_cursor( m_pConnection, m_Window );
			xcb_unmap_window( m_pConnection, m_Window );
			xcb_destroy_window( m_pConnection, m_Window );
			xcb_disconnect( m_pConnection );
			m_pConnection = nullptr;
		}

		m_Open = False;
	}

	void GameWindowXCB::ProcessEvents( )
	{
		xcb_generic_event_t *pEvent;

		while( ( pEvent = xcb_poll_for_event( m_pConnection ) ) != nullptr )
		{
			switch( pEvent->response_type & 0x7F )
			{
				case XCB_CONFIGURE_NOTIFY:
				{
					xcb_configure_notify_event_t *pConfigureEvent =
						reinterpret_cast< xcb_configure_notify_event_t * >(
							pEvent );

					if( ( ( pConfigureEvent->width > 0 ) &&
							( m_Width != pConfigureEvent->width ) ) ||
						( ( pConfigureEvent->height > 0 ) &&
							( m_Height != pConfigureEvent->height ) ) )
					{
						m_Width = pConfigureEvent->width;
						m_Height = pConfigureEvent->height;
						m_Resize = True;
					}

					break;
				}
				case XCB_CLIENT_MESSAGE:
				{
					if( ( *( xcb_client_message_event_t * )
							pEvent ).data.data32[ 0 ] ==
								( *m_pDeleteReply ).atom )
					{
						std::cout << "Window closing" << std::endl;
						m_Open = False;
						free( m_pDeleteReply );
						m_pDeleteReply = nullptr;

						break;
					}
					break;
				}
				case XCB_KEY_PRESS:
				{
					m_Open = False;
					break;
				}
			}
		}

		if( m_Resize == True )
		{
			m_pRenderer->TriggerResize( );
			m_Resize = False;
		}
	}
}

