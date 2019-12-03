#include <DataTypes.hpp>
#include <sstream>

namespace Brawl
{
	ErrorCode GetCompilerString( std::string &p_CompilerString )
	{
		std::ostringstream StringStream;
		StringStream << "GNU Compiler Collection C++ (" << __VERSION__ << ")";

		p_CompilerString = StringStream.str( );

		return ErrorCode::Okay;
	}
}
