#include "rendering/ShaderProcessor.hpp"

#include "Logger.hpp"
#include "Util.hpp"

#include <sstream>
#include <string_view>
#include <vector>

namespace leopph::internal
{
	auto ProcessShader(ShaderSourceFileInfo const& fileInfo) -> std::string
	{
		auto changed = true;
		auto processedSrc = fileInfo.content;

		while (changed)
		{
			changed = false;

			static std::vector<std::string> lines;
			lines.clear();
			SplitLines(processedSrc, lines);

			std::ostringstream outStream;

			for (std::size_t i = 0; i < lines.size(); i++)
			{
				std::string_view view{lines[i]};

				auto constexpr whiteSpaceChars = "\t\n\v\f\r ";
				auto numWhiteSpace = view.find_first_not_of(whiteSpaceChars);

				if (numWhiteSpace != std::string_view::npos)
				{
					view.remove_prefix(numWhiteSpace);

					if (view.starts_with('#'))
					{
						view.remove_prefix(1);

						numWhiteSpace = view.find_first_not_of(whiteSpaceChars);

						if (numWhiteSpace == std::string_view::npos)
						{
							Logger::Instance().Error("Incomplete preprocessor directive on line " + std::to_string(i) + '.');
							return {};
						}

						view.remove_prefix(numWhiteSpace);

						if (view.starts_with("include"))
						{
							view.remove_prefix(7);

							numWhiteSpace = view.find_first_not_of(whiteSpaceChars);

							if (numWhiteSpace == std::string_view::npos)
							{
								Logger::Instance().Error("Incomplete include directive on line " + std::to_string(i) + ": missing file specifier.");
								return {};
							}

							view.remove_prefix(numWhiteSpace);

							char fileSpecClosingChar;

							if (view.starts_with('<'))
							{
								fileSpecClosingChar = '>';
							}
							else if (view.starts_with('\"'))
							{
								fileSpecClosingChar = '\"';
							}
							else
							{
								Logger::Instance().Error("Ill-formed include directive on line " + std::to_string(i) + ": unknown token in place of file specifier.");
								return {};
							}

							view.remove_prefix(1);

							auto const filenameLngth = view.find_first_of(fileSpecClosingChar);

							if (filenameLngth == std::string_view::npos)
							{
								Logger::Instance().Error("Ill-formed include directive on line " + std::to_string(i) + ": missing file specifier closer.");
								return {};
							}

							auto const inclFilename = [&view, filenameLngth, &fileInfo]
							{
								std::filesystem::path ret{view.substr(0, filenameLngth)};

								if (ret.is_relative())
								{
									ret = fileInfo.absolutePath.parent_path() / ret;
								}

								ret.make_preferred();
								return ret;
							}();

							lines[i] = ReadFile(inclFilename);
							changed = true;
						}
					}
				}

				outStream << lines[i] << '\n';
			}

			processedSrc = outStream.str();
		}

		return processedSrc;
	}
}
