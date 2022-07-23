#include "ShaderProcessing.hpp"

#include "Logger.hpp"
#include "RenderSettings.hpp"
#include "Util.hpp"

#include <format>
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



	namespace
	{
		void process_includes_recursive(std::filesystem::path const& absolutePath, std::vector<std::string>& lines)
		{
			std::regex const static includeLineRegex{R"delim(^\s*#\s*include\s*)delim"};
			std::regex const static includeLineQuoteRegex{R"delim(^\s*#\s*include\s*"\S+"\s*$)delim"};
			std::regex const static includeLineBracketRegex{R"delim(^\s*#\s*include\s*<\S+>\s*$)delim"};
			std::regex const static fileNameInQuotesRegex{R"delim("\S+")delim"};
			std::regex const static fileNameInBracketsRegex{R"delim(<\S+>)delim"};

			std::vector<std::string> inclFileBuf;

			// Index based loop because we will potentially insert lines into the vector
			for (std::size_t i{0}; i < lines.size(); i++)
			{
				// starts with #include
				if (std::regex_search(lines[i], includeLineRegex))
				{
					std::string includeFileName;

					auto const getFileNameFromLine = [i, &lines](std::regex const& nameRegex) -> std::string
					{
						std::regex_iterator const regIt{std::begin(lines[i]), std::end(lines[i]), nameRegex};
						return regIt->str().substr(1, regIt->length() - 2);
					};

					// #include "file"
					if (std::regex_match(lines[i], includeLineQuoteRegex))
					{
						includeFileName = getFileNameFromLine(fileNameInQuotesRegex);
					}
					// #include <file>
					else if (std::regex_match(lines[i], includeLineBracketRegex))
					{
						includeFileName = getFileNameFromLine(fileNameInBracketsRegex);
					}
					else
					{
						// We couldn't parse the include directive so we log the error, clear the erroneous lines[i], and move on.
						internal::Logger::Instance().Error("Error parsing shader includes: ill-formed file specifier was found.");
						lines[i].clear();
						i++;
						continue;
					}

					auto const includePath = absolute(absolutePath.parent_path() / includeFileName).make_preferred();

					inclFileBuf.clear();
					read_file_lines(includePath, inclFileBuf);

					if (inclFileBuf.empty())
					{
						// The the file to be included was either empty, or does not exist.
						internal::Logger::Instance().Error(std::format("Error parsing shader includes: included file [{}] was empty or does not exist.", includePath.string()));
					}

					process_includes_recursive(includePath, inclFileBuf);

					// Clear include lines[i]
					lines[i].clear();

					// Add included file lines at the include directive's position
					lines.insert(std::begin(lines) + i, std::make_move_iterator(std::begin(inclFileBuf)), std::make_move_iterator(std::end(inclFileBuf)));

					// We don't increment here because the current index points to the first lines[i] of the included file, which we are yet to examine
				}
			}
		}
	}



	void process_shader_includes(ShaderProgramSourceFileInfo sourceFileInfo, ShaderProgramSourceLines& out)
	{
		process_includes_recursive(sourceFileInfo.absolutePath, sourceFileInfo.lines);
		out = std::move(sourceFileInfo.lines);
	}
}
