#include "Leopphverter.hpp"

#include <iostream>


namespace leopph::convert::driver
{
	auto ParseInteractive(std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames) -> void
	{
		std::cout << "Interactive mode. Enter $DONE$ any time to start processing the input data.\n";
		std::cout << "To keep the original file names, leave the corresponding prompt empty.\n\n";

		std::string line;

		while (true)
		{
			std::cout << "Path to model file: ";
			do
			{
				std::getline(std::cin, line);
			}
			while (line.empty());

			if (line == "$DONE$")
			{
				break;
			}

			filesToConvert.emplace_back(line);

			std::cout << "Output file name (optional): ";
			std::getline(std::cin, line);

			if (!line.empty())
			{
				if (line == "$DONE$")
				{
					break;
				}

				outputFileNames.push_back(line);
			}
		}

		std::cout << '\n';
	}
}
