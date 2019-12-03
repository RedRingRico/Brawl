#include <NativeFile.hpp>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

namespace Brawl
{
	int ConvertSeekPositionToWhence( const SeekPosition p_Position )
	{
		if( p_Position == SeekPosition::Set )
		{
			return SEEK_SET;
		}

		if( p_Position == SeekPosition::Current )
		{
			return SEEK_CUR;
		}

		if( p_Position == SeekPosition::End )
		{
			return SEEK_END;
		}

		return -1;
	}

	NativeFile::NativeFile( ) :
		m_FileHandle( InvalidFileHandle ),
		m_FileDescriptor( InvalidFileDescriptor )
	{
	}

	NativeFile::~NativeFile( )
	{
		Close( );
	}

	ErrorCode NativeFile::Open( const std::string &p_FileName,
		const AccessFlags p_AccessFlags )
	{
		std::string FileMode;

		if( ConvertAccessFlagsToPOSIXMode( p_AccessFlags, FileMode ) !=
			ErrorCode::Okay )
		{
			return ErrorCode::InvalidFileAccessFlags;
		}

		m_FileHandle = fopen64( p_FileName.c_str( ), FileMode.c_str( ) );

		if( m_FileHandle == InvalidFileHandle )
		{
			return ErrorCode::InvalidFileHandle;
		}

		m_FileDescriptor = fileno( m_FileHandle );

		m_Size = Seek( 0, SeekPosition::End );
		Rewind( );

		SetAccessFlags( p_AccessFlags );
		++m_ReferenceCount;

		return ErrorCode::Okay;
	}

	ErrorCode NativeFile::Close( )
	{
		if( m_FileHandle != InvalidFileHandle )
		{
			if( --m_ReferenceCount == 0UL )
			{
				std::cout << "Closing..." << std::endl;
				fclose( m_FileHandle );

				m_FileHandle = InvalidFileHandle;
				m_FileDescriptor = InvalidFileDescriptor;
			}
		}

		return ErrorCode::Okay;
	}

	int64_t NativeFile::Seek( const int64_t p_Offset,
		const SeekPosition p_Start )
	{
		if( m_FileDescriptor != InvalidFileDescriptor )
		{
			int Whence = ConvertSeekPositionToWhence( p_Start );

			if( Whence != -1 )
			{
				off64_t SeekResult = lseek64( m_FileDescriptor, p_Offset,
					Whence );

				if( SeekResult != -1 )
				{
					m_Position = SeekResult;

					return SeekResult;
				}

				// Really need to handle these...
				switch( errno )
				{
					case EBADF:
					case EINVAL:
					case EOVERFLOW:
					case ESPIPE:
					default:
					{
					}
				}
			}
		}

		return -1;
	}

	ErrorCode NativeFile::Rewind( )
	{
		if( Seek( 0, SeekPosition::Set ) != 0 )
		{
			return ErrorCode::FileRewindFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode NativeFile::ReadBytes( Byte *p_pData,
		const uint64_t p_Count, uint64_t &p_Read )
	{
		p_Read = fread( p_pData, 1, p_Count, m_FileHandle );

		if( p_Read != p_Count )
		{
			return ErrorCode::ReadBytesFailed;
		}

		return ErrorCode::Okay;
	}

	ErrorCode NativeFile::WriteBytes( const Byte * const p_pData,
		const uint64_t p_Count, uint64_t &p_Written )
	{
		size_t Written = fwrite( p_pData, 1, p_Count, m_FileHandle );

		if( Written != p_Count )
		{
			return ErrorCode::WriteBytesFailed;
		}

		return ErrorCode::Okay;
	}
}

