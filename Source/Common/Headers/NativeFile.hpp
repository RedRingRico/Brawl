#ifndef __BRAWL_NATIVEFILE_HPP__
#define __BRAWL_NATIVEFILE_HPP__

#include <DataTypes.hpp>
#include <File.hpp>

namespace Brawl
{
	class NativeFile : public File
	{
	public:
		NativeFile( );
		~NativeFile( );

		ErrorCode Open( const std::string &p_FileName,
			const AccessFlags p_AccessFlags ) override;
		ErrorCode Close( ) override;

		int64_t Seek( const int64_t p_Offset,
			const SeekPosition p_Start = SeekPosition::Current ) override;
		ErrorCode Rewind( ) override;

		ErrorCode ReadBytes( Byte *p_pData,
			const uint64_t p_Count, uint64_t &p_Read ) override;

		ErrorCode WriteBytes( const Byte * const p_pData,
			const uint64_t p_Count, uint64_t &p_Written ) override;

	private:
		FileHandle		m_FileHandle;
		FileDescriptor	m_FileDescriptor;
	};
}

#endif // __BRAWL_NATIVEFILE_HPP__

