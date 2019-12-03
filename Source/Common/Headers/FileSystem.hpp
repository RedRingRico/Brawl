#ifndef __BRAWL_FILESYSTEM_HPP__
#define __BRAWL_FILESYSTEM_HPP__

#include <DataTypes.hpp>
#include <ErrorCodes.hpp>
#include <File.hpp>

#include <map>
#include <set>
#include <string>

namespace Brawl
{
	class FileContainer;

	class FileSystem
	{
	public:
		static FileSystem &Get( )
		{
			static FileSystem Instance;
			return Instance;
		}

		// Disable copy, move, and assignment
		FileSystem( const FileSystem &p_Copy ) = delete;
		void operator=( const FileSystem &p_Clone ) = delete;

		ErrorCode Initialise( );

		ErrorCode Open( const std::string &p_FileName,
			const AccessFlags p_AccessFlags, File **p_ppFile );

	private:
		FileSystem( ) { }

		ErrorCode AllocateFileContainers( );
		void DeallocateFileContainers( );

		Bool							m_Initialised;
		std::set< FileContainer * >		m_FileContainers;
		//std::map< std::string, File * >	m_FileMap;
		std::map< uint32_t, File * >	m_FileMap;
	};
}

#endif // __BRAWL_FILESYSTEM_HPP__
