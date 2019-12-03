#include <FileSystem.hpp>
#include <FileContainer.hpp>
#include <NativeFileContainer.hpp>
#include <iostream>

namespace Brawl
{
	const uint32_t FNV1A_PRIME = 0x01000193;
	const uint32_t FNV1A_SEED = 0x811C9DC5;

	uint32_t FNV1a( uint8_t p_Byte, uint32_t p_Hash = FNV1A_SEED )
	{
		return ( p_Byte ^ p_Hash ) * FNV1A_PRIME;
	}

	uint32_t FNV1a( const void *p_pData, size_t p_ByteCount, uint32_t p_Hash = FNV1A_SEED )
	{
		const uint8_t *pPtr = reinterpret_cast< const uint8_t * >( p_pData );

		while( p_ByteCount-- )
		{
			p_Hash = ( *pPtr++ ^ p_Hash ) * FNV1A_PRIME;
		}

		return p_Hash;
	}
	
	uint32_t HashString( const char *p_pString, uint32_t p_Hash = FNV1A_SEED )
	{
		while( *p_pString )
		{
			p_Hash = FNV1a( ( uint8_t )*p_pString++, p_Hash );
		}

		return p_Hash;
	}

	uint32_t HashString( const std::string &p_String, uint32_t p_Hash = FNV1A_SEED )
	{
		return FNV1a( p_String.c_str( ), p_String.length( ), p_Hash );
	}

	ErrorCode FileSystem::Initialise( )
	{
		if( m_Initialised == false )
		{
			ErrorCode Error = AllocateFileContainers( );

			if( Error == ErrorCode::Okay )
			{
				m_Initialised = true;
				return Error;
			}
		}

		return ErrorCode::FileSystemAlreadyInitialised;
	}

	ErrorCode FileSystem::Open( const std::string &p_FileName,
		const AccessFlags p_AccessFlags, File **p_ppFile )
	{
		if( m_Initialised == False )
		{
			ErrorCode InitialiseError = this->Initialise( );

			if( InitialiseError != ErrorCode::Okay )
			{
				return InitialiseError;
			}
		}

		( *p_ppFile ) = Null;

		// Is the file already open with the same access flags?
		uint32_t FileNameHash = HashString( p_FileName );
		const auto &FileFound = m_FileMap.find( FileNameHash );

		if( FileFound != m_FileMap.end( ) )
		{
			if( FileFound->second->IsOpen( ) &&
				FileFound->second->GetAccessFlags( ) == p_AccessFlags )
			{
				std::cout << "FOUND FILE, RETURNING" << std::endl;
				( *p_ppFile ) = FileFound->second;
				FileFound->second->AddReference( );

				return ErrorCode::Okay;
			}
		}

		// Deduce the URI and use the appropriate file container to open it
		size_t Colon = p_FileName.find_first_of( ':' );

		if( Colon == std::string::npos )
		{
			return ErrorCode::MalformedFileURI;
		}

		std::string URI = p_FileName.substr( 0, Colon );

		auto FileContainerItr = m_FileContainers.begin( );

		for( ; FileContainerItr != m_FileContainers.end( );
			++FileContainerItr )
		{
			std::string URIHandler;

			if( ( *FileContainerItr )->GetURIHandler( URIHandler ) ==
				ErrorCode::Okay )
			{
				if( URIHandler.compare( URI ) == 0 )
				{
					break;
				}
			}
		}

		if( FileContainerItr == m_FileContainers.end( ) )
		{
			return ErrorCode::UnknownFileURI;
		}

		std::string FilePath = p_FileName.substr( Colon + 1 );

		if( ( *FileContainerItr )->Open( FilePath, p_AccessFlags,
			p_ppFile ) != ErrorCode::Okay )
		{
			return ErrorCode::OpenFileFailed;
		}

		m_FileMap[ FileNameHash ] =  ( *p_ppFile );

		return ErrorCode::Okay;
	}

	ErrorCode FileSystem::AllocateFileContainers( )
	{
		// Register file container types
		NativeFileContainer *pNFC = new NativeFileContainer( );
		m_FileContainers.emplace( pNFC );

		return ErrorCode::Okay;
	}

	void FileSystem::DeallocateFileContainers( )
	{
		for( auto Container : m_FileContainers )
		{
			if( Container )
			{
				delete Container;
			}
		}

		m_FileContainers.clear( );
	}
}

