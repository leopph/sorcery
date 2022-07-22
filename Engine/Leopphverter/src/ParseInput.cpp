#include "ParseInput.hpp"

#include "Logger.hpp"
#include "Types.hpp"
#include "Util.hpp"

#include <format>
#include <iostream>


namespace leopph::convert::driver
{
	namespace
	{
		constexpr auto OUT_FILENAME_SEP = ';';
	}



	int parse_command_line(int const argc, char const** argv, std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames)
	{
		std::cout << "Using command line arguments.\n\n";

		std::vector<std::string_view> buf;

		// 0: Reading input filenames or option keys
		// 1: Reading a semicolon separated list of output filenames
		u8 currentParseMode{0};

		for (auto i = 1; i < argc; i++)
		{
			switch (currentParseMode)
			{
				// Reading input filenames or options keys
				case 0:
				{
					// The str is an option key
					if (argv[i][0] == '-')
					{
						switch (argv[i][1])
						{
							// output key
							case 'o':
							{
								currentParseMode = 1;
								break;
							}

							default:
							{
								internal::Logger::Instance().Error(std::format("Unknown option [{}].", argv[i]));
								return EXIT_FAILURE;
							}
						}
					}
					// The str is a filename
					else
					{
						filesToConvert.emplace_back(argv[i]);
					}
					break;
				}

				// Reading output filenames
				case 1:
				{
					buf.clear();
					split_string_delim(argv[i], OUT_FILENAME_SEP, buf);
					outputFileNames.insert(std::end(outputFileNames), std::begin(buf), std::end(buf));
					currentParseMode = 0;
					break;
				}

				default:
				{
					internal::Logger::Instance().Debug("Parse mode was invalid.");
					internal::Logger::Instance().Error("Internal error.");
					return EXIT_FAILURE;
				}
			}
		}

		return EXIT_SUCCESS;
	}



	void parse_interactive(std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames)
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
