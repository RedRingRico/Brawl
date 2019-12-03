#ifndef __BRAWL_NATIVEFILECONTAINER_HPP__
#define __BRAWL_NATIVEFILECONTAINER_HPP__

#include <FileContainer.hpp>
#include <NativeFile.hpp>

namespace Brawl
{
	class NativeFileContainer : public FileContainer
	{
	public:
		NativeFileContainer( );
		~NativeFileContainer( );

		ErrorCode Open( const std::string &p_FileName,
			const AccessFlags p_AccessFlags, File **p_ppFile ) override;
	};
}

#endif // __BRAWL_NATIVEFILECONTAINER_HPP__

