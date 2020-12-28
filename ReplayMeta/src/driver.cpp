#include <iostream>
#include <vector>

#include "ReplayMeta.h"

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cerr << "No file specified.";
	}

	std::vector<ReplayMeta> replayList = ReplayMeta::create_replaymeta_list(argv[1]);

	for (auto test : replayList)
	{
		std::cout << test.string() << std::endl;
	}
	
	return 0;
}