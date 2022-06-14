#include "parse_file.h"

#include <fstream>
#include <sstream>

namespace parser
{
	std::string ParseFile(std::filesystem::path path) noexcept
	{
		std::string ret;
		std::ifstream fileStream{ path };
		std::stringstream collectorStream;

		collectorStream << fileStream.rdbuf();
		fileStream.close();

		return collectorStream.str();
	}
}