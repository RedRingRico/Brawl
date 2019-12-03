#ifndef __BRAWL_FILE_HPP__
#define __BRAWL_FILE_HPP__

#include <DataTypes.hpp>
#include <ErrorCodes.hpp>

#include <string>
#include <type_traits>

namespace Brawl
{
	const uint32_t AccessFlagsBase = 1;

	enum class AccessFlags : uint32_t
	{
		Read = AccessFlagsBase,
		Write = AccessFlagsBase << 1,
		Append = AccessFlagsBase << 2,
		Update = AccessFlagsBase << 3,
		Binary = AccessFlagsBase << 4,
		Text = AccessFlagsBase << 5
	};

	using AccessFlagsType = std::underlying_type_t< AccessFlags >;

	inline bool operator&( AccessFlags p_Left, AccessFlags p_Right )
	{
		return static_cast< bool >(
			( static_cast< AccessFlagsType >( p_Left ) &
				 static_cast< AccessFlagsType >( p_Right ) ) );
	}

	inline AccessFlags operator|( AccessFlags p_Left, AccessFlags p_Right )
	{
		return static_cast< AccessFlags >(
			( static_cast< AccessFlagsType >( p_Left ) |
				 static_cast< AccessFlagsType >( p_Right ) ) );
	}

	inline AccessFlags &operator|=( AccessFlags &p_Left, AccessFlags p_Right )
	{
		p_Left = p_Left | p_Right;

		return p_Left;
	}

	const uint32_t FileOpenFlag = 0x11;

	enum class SeekPosition : uint8_t
	{
		Set = 0,
		Current = 1,
		End = 2
	};

	ErrorCode ConvertAccessFlagsToPOSIXMode( const AccessFlags p_AccessFlags,
		std::string &p_Mode );

	class File
	{
	public:
		File( );
		virtual ~File( );

		virtual ErrorCode Open( const std::string &p_FileName,
			const AccessFlags p_AccessFlags ) = 0;
		virtual ErrorCode Close( ) = 0;

		// Position manipulation
		virtual int64_t Seek( const int64_t p_Offset,
			const SeekPosition p_Start = SeekPosition::Current ) = 0;
		virtual ErrorCode Rewind( ) = 0;

		// Read functions
		virtual ErrorCode ReadBytes( Byte *p_pData, const uint64_t p_Count,
			uint64_t &p_Read ) = 0;

		// Write functions
		virtual ErrorCode WriteBytes( const Byte * const p_pData,
			const uint64_t p_Count, uint64_t &p_Written ) = 0;

		// Common functions that do not need to be pure virtual
		int64_t GetSize( ) const;
		int64_t GetPosition( ) const;
		Bool IsOpen( ) const;
		AccessFlags GetAccessFlags( ) const;
		void AddReference( );

	protected:
		void SetAccessFlags( const AccessFlags p_Flags );

		int64_t		m_Size;
		int64_t		m_Position;
		uint32_t	m_Flags;
		uint32_t	m_ReferenceCount;
	};
}

#endif // __BRAWL_FILE_HPP__

