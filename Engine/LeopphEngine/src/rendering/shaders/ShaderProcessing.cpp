#include "rendering/shaders/ShaderProcessing.hpp"

#include "Logger.hpp"
#include "Util.hpp"

#include <regex>
#include <stdexcept>
#include <iterator>

namespace leopph
{
	auto ReadShaderFiles(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath) -> ShaderProgramSourceFileInfo
	{
		if (vertexShaderPath.empty())
		{
			auto const errMsg = "Error reading shader files: vertex shader path was empty. Vertex shaders are not optional.";
			internal::Logger::Instance().Error(errMsg);
			throw std::invalid_argument{errMsg};
		}

		ShaderProgramSourceFileInfo programFileInfo;

		std::vector<std::string> fileLines;

		vertexShaderPath = absolute(vertexShaderPath).make_preferred();
		ReadFileLines(vertexShaderPath, fileLines);
		programFileInfo.vertexInfo = {std::move(vertexShaderPath), std::move(fileLines)};

		if (!geometryShaderPath.empty())
		{
			geometryShaderPath = absolute(geometryShaderPath).make_preferred();
			ReadFileLines(geometryShaderPath, fileLines);
			programFileInfo.geometryInfo = {std::move(geometryShaderPath), std::move(fileLines)};
		}

		if (!fragmentShaderPath.empty())
		{
			fragmentShaderPath = absolute(fragmentShaderPath).make_preferred();
			ReadFileLines(fragmentShaderPath, fileLines);
			programFileInfo.fragmentInfo = {std::move(fragmentShaderPath), std::move(fileLines)};
		}

		return programFileInfo;
	}

	auto ProcessShaderIncludes(ShaderProgramSourceFileInfo sourceFileInfo) -> ShaderProgramSourceInfo
	{
		std::regex const static includeLineRegex{R"delim(^\s*#\s*include\s*)delim"};
		std::regex const static includeLineQuoteRegex{R"delim(^\s*#\s*include\s*"\S+"\s*$)delim"};
		std::regex const static includeLineBracketRegex{R"delim(^\s*#\s*include\s*<\S+>\s*$)delim"};
		std::regex const static fileNameInQuotesRegex{R"delim("\S+")delim"};
		std::regex const static fileNameInBracketsRegex{R"delim(<\S+>)delim"};

		ShaderProgramSourceInfo programSourceInfo;

		if (!sourceFileInfo.vertexInfo)
		{
			auto const errMsg = "Error processing shader includes: vertex shader info was missing. Vertex shaders are not optional.";
			internal::Logger::Instance().Error(errMsg);
			throw std::invalid_argument{errMsg};
		}

		std::vector<ShaderStageSourceFileInfo*> stageInfos;
		stageInfos.push_back(&*sourceFileInfo.vertexInfo);

		if (sourceFileInfo.geometryInfo)
		{
			stageInfos.push_back(&*sourceFileInfo.geometryInfo);
		}

		if (sourceFileInfo.fragmentInfo)
		{
			stageInfos.push_back(&*sourceFileInfo.fragmentInfo);
		}

		std::vector<std::string> includedLines;

		for (auto* const stageInfo : stageInfos)
		{
			bool doAnotherPass;

			do
			{
				doAnotherPass = false;

				for (auto lineIt = std::begin(stageInfo->fileContents); lineIt != std::end(stageInfo->fileContents); ++lineIt)
				{
					if (auto& line = *lineIt; std::regex_search(line, includeLineRegex))
					{
						std::string includeFileName;

						if (std::regex_match(line, includeLineQuoteRegex))
						{
							std::regex_iterator regIt{std::begin(line), std::end(line), fileNameInQuotesRegex};
							includeFileName = regIt->str().substr(regIt->length() - 2);
						}
						else if (std::regex_match(line, includeLineBracketRegex))
						{
							std::regex_iterator regIt{std::begin(line), std::end(line), fileNameInBracketsRegex};
							includeFileName = regIt->str().substr(regIt->length() - 2);
						}
						else
						{
							auto const errMsg = "Error parsing shader includes: ill-formed file specifier was found.";
							internal::Logger::Instance().Error(errMsg);
							throw std::runtime_error{errMsg};
						}

						auto const includePath = absolute(stageInfo->absolutePath.parent_path() / includeFileName).make_preferred();

						includedLines.clear();
						ReadFileLines(includePath, includedLines);

						stageInfo->fileContents.erase(lineIt);
						stageInfo->fileContents.insert(lineIt, std::make_move_iterator(std::begin(includedLines)), std::make_move_iterator(std::end(includedLines)));

						doAnotherPass = true;
						break;
					}
				}
			}
			while (doAnotherPass);
		}

		return programSourceInfo;
	}
}
