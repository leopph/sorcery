#include "ShaderProcessing.hpp"

#include "Logger.hpp"
#include "RenderSettings.hpp"
#include "Util.hpp"

#include <iterator>
#include <regex>



namespace leopph
{
	bool read_shader_files(std::filesystem::path const& filePath, ShaderProgramSourceFileInfo& out)
	{
		if (filePath.empty())
		{
			internal::Logger::Instance().Error("Error reading shader file: shader path was empty.");
			return false;
		}

		if (!exists(filePath))
		{
			internal::Logger::Instance().Error("Error reading shader file: the file does not exist.");
			return false;
		}

		out.absolutePath = absolute(filePath).make_preferred();
		out.lines.clear();
		read_file_lines(out.absolutePath, out.lines);

		return true;
	}



	void process_shader_includes(ShaderProgramSourceFileInfo sourceFileInfo, ShaderProgramSourceLines& out)
	{
		std::regex const static includeLineRegex{R"delim(^\s*#\s*include\s*)delim"};
		std::regex const static includeLineQuoteRegex{R"delim(^\s*#\s*include\s*"\S+"\s*$)delim"};
		std::regex const static includeLineBracketRegex{R"delim(^\s*#\s*include\s*<\S+>\s*$)delim"};
		std::regex const static fileNameInQuotesRegex{R"delim("\S+")delim"};
		std::regex const static fileNameInBracketsRegex{R"delim(<\S+>)delim"};

		std::vector<std::string> inclFileBuf;

		// Index based loop because we will potentially insert lines into the vector
		for (std::size_t lineIndex{0}; lineIndex < sourceFileInfo.lines.size();)
		{
			// starts with #include
			if (auto& line = sourceFileInfo.lines[lineIndex]; std::regex_search(line, includeLineRegex))
			{
				std::string includeFileName;

				auto const getFileNameFromLine = [](std::string_view const line, std::regex const& nameRegex) -> std::string
				{
					std::regex_iterator const regIt{std::begin(line), std::end(line), nameRegex};
					return regIt->str().substr(1, regIt->length() - 2);
				};

				// #include "file"
				if (std::regex_match(line, includeLineQuoteRegex))
				{
					includeFileName = getFileNameFromLine(line, fileNameInQuotesRegex);
				}
				// #include <file>
				else if (std::regex_match(line, includeLineBracketRegex))
				{
					includeFileName = getFileNameFromLine(line, fileNameInBracketsRegex);
				}
				else
				{
					// We couldn't parse the include directive so we log the error, clear the erroneous line, and move on.
					internal::Logger::Instance().Error("Error parsing shader includes: ill-formed file specifier was found.");
					line.clear();
					lineIndex++;
					continue;
				}

				auto const includePath = absolute(sourceFileInfo.absolutePath.parent_path() / includeFileName).make_preferred();

				inclFileBuf.clear();
				read_file_lines(includePath, inclFileBuf);

				// Clear include line
				line.clear();

				// Add included file lines at the include directive's position
				sourceFileInfo.lines.insert(std::begin(sourceFileInfo.lines) + lineIndex, std::make_move_iterator(std::begin(inclFileBuf)), std::make_move_iterator(std::end(inclFileBuf)));

				// We don't increment here because the current index points to the first line of the included file, which we are yet to examine
			}
			else
			{
				lineIndex++;
			}
		}

		out = std::move(sourceFileInfo.lines);
	}
}
