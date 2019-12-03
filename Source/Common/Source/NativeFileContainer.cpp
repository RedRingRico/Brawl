#include <NativeFileContainer.hpp>
#include <NativeFile.hpp>
#include <iostream>

namespace Brawl
{
	NativeFileContainer::NativeFileContainer( ) :
		FileContainer( )
	{
		m_URIHandler = "native";
	}

	NativeFileContainer::~NativeFileContainer( )
	{
	}

	ErrorCode NativeFileContainer::Open( const std::string &p_FileName,
		const AccessFlags p_AccessFlags, File **p_ppFile )
	{
		( *p_ppFile ) = new NativeFile( );

		if( ( *p_ppFile )->Open( p_FileName, p_AccessFlags ) !=
			ErrorCode::Okay )
		{
			return ErrorCode::NativeFileContainerOpenFailed;
		}

		return ErrorCode::Okay;
	}
}

