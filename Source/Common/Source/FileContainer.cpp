#include <FileContainer.hpp>

namespace Brawl
{
	FileContainer::FileContainer( )
	{
	}

	FileContainer::~FileContainer( )
	{
	}

	ErrorCode FileContainer::GetURIHandler( std::string &p_URIHandler )
	{
		if( m_URIHandler.size( ) > 0 )
		{
			p_URIHandler = m_URIHandler;

			return ErrorCode::Okay;
		}

		return ErrorCode::FileURIHandlerUndefined;
	}
}

