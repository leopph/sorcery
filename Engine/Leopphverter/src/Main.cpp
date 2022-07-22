#include "Leopphverter.hpp"
#include "LeopphverterExport.hpp"
#include "LeopphverterImport.hpp"
#include "Logger.hpp"

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <vector>


int main(int const argc, char const** const argv)
{
	std::cout << leopph::convert::driver::BuildHeader() << '\n';

	std::vector<std::filesystem::path> filesToConvert;
	std::vector<std::string> outputFileNames;

	// We got command line arguments, we're parsing
	if (argc > 1)
	{
		if (auto const res = leopph::convert::driver::ParseCommandLine(argc, argv, filesToConvert, outputFileNames); res != EXIT_SUCCESS)
		{
			return res;
		}
	}
	// We instead ask for files through the command line
	else
	{
		leopph::convert::driver::ParseInteractive(filesToConvert, outputFileNames);
	}

	for (leopph::u64 i = 0; i < filesToConvert.size(); i++)
	{
		auto const model = leopph::convert::Import(filesToConvert[i]);

		if (!model.has_value())
		{
			std::cerr << "Skipping " << filesToConvert[i] << " because of a failed import.\n";
			continue;
		}

		auto bytes = Export(model.value(), std::endian::native);

		auto const outputName = [i, &filesToConvert, &outputFileNames]() -> std::string
		{
			if (i < outputFileNames.size() && !outputFileNames[i].empty())
			{
				filesToConvert[i].replace_filename(outputFileNames[i]);
			}

			filesToConvert[i].replace_extension("leopph3d");
			return filesToConvert[i].string();
		}();

		std::ofstream out{outputName, std::ios::binary | std::ios::out};
		out.setf(std::ios_base::unitbuf);
		out.write(reinterpret_cast<char const*>(bytes.data()), bytes.size());

		std::cout << filesToConvert[i].string() << " is completed.\n";
	}
}
