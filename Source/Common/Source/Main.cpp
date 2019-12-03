#include <iostream>
#include <string>
#include <GameProgram.hpp>

int main( int p_Argc, char **p_ppArgv )
{
	std::cout << "Brawl" << std::endl;

	std::string CompilerVersion;

	Brawl::GetCompilerString( CompilerVersion );

	std::cout << CompilerVersion << std::endl;

	Brawl::GameProgram BrawlGame;

	if( BrawlGame.Initialise( ) != Brawl::ErrorCode::Okay )
	{
		std::cout << "Failed to start the game" << std::endl;

		return 1;
	}

	if( BrawlGame.Execute( ) != Brawl::ErrorCode::Okay )
	{
		std::cout << "Failed to execute the game" << std::endl;

		return 1;
	}

	return 0;
}

