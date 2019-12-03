#ifndef __BRAWL_FILECONTAINER_HPP__
#define __BRAWL_FILECONTAINER_HPP__

#include <DataTypes.hpp>
#include <ErrorCodes.hpp>
#include <File.hpp>

#include <string>

namespace Brawl
{
	class FileContainer
	{
	public:
		FileContainer( );
		virtual ~FileContainer( );

		virtual ErrorCode Open( const std::string &p_FileName,
			const AccessFlags p_AccessFlags, File **p_pFile ) = 0;

		ErrorCode GetURIHandler( std::string &p_URIHandler );

	protected:
		std::string	m_URIHandler;
	};
}

#endif // __BRAWL_FILECONTAINER_HPP__

