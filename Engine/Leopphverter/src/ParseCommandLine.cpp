#include "Common.hpp"
#include "Leopphverter.hpp"
#include "Logger.hpp"
#include "Types.hpp"

#include <cstdlib>
#include <iostream>


namespace leopph::convert::driver
{
	int ParseCommandLine(int const argc, char const** argv, std::vector<std::filesystem::path>& filesToConvert, std::vector<std::string>& outputFileNames)
	{
		std::cout << "Using command line arguments.\n\n";

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
								using std::literals::string_literals::operator ""s;
								internal::Logger::Instance().Error("Unknown option: "s + argv[i]);
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
					auto const split = SplitString(argv[i], OUT_FILENAME_SEP);
					outputFileNames.insert(std::end(outputFileNames), std::begin(split), std::end(split));
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
}
