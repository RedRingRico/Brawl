#include <File.hpp>

namespace Brawl
{
	ErrorCode ConvertAccessFlagsToPOSIXMode( const AccessFlags p_AccessFlags,
		std::string &p_Mode )
	{
		if( p_AccessFlags & ( AccessFlags::Binary | AccessFlags::Text ) )
		{
			return ErrorCode::InvalidFileAccessFlags;
		}

		if( p_AccessFlags & AccessFlags::Read )
		{
			p_Mode.append( "r" );
		}

		if( p_AccessFlags & AccessFlags::Write )
		{
			p_Mode.append( "w" );
		}

		if( p_AccessFlags & AccessFlags::Append )
		{
			p_Mode.append( "a" );
		}

		if( p_AccessFlags & AccessFlags::Update )
		{
			p_Mode.append( "+" );
		}

		if( p_AccessFlags & AccessFlags::Binary )
		{
			p_Mode.append( "b" );
		}

		return ErrorCode::Okay;
	}

	File::File( ) :
		m_Size( 0LL ),
		m_Position( 0LL ),
		m_Flags( 0UL ),
		m_ReferenceCount( 0UL )
	{
	}

	File::~File( )
	{
	}

	int64_t File::GetSize( ) const
	{
		return m_Size;
	}

	int64_t File::GetPosition( ) const
	{
		return m_Position;
	}

	Bool File::IsOpen( ) const
	{
		return m_Flags & FileOpenFlag ? True : False;
	}

	AccessFlags File::GetAccessFlags( ) const
	{
		// Lowest six bits contain the access flags
		return static_cast< AccessFlags >( m_Flags & 0x3F );
	}

	void File::AddReference( )
	{
		++m_ReferenceCount;
	}

	void File::SetAccessFlags( const AccessFlags p_Flags )
	{
		m_Flags |= ( static_cast< uint32_t >( p_Flags ) & 0x3F );
	}
}

