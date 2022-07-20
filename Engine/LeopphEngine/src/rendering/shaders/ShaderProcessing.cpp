#include "rendering/shaders/ShaderProcessing.hpp"

#include "Logger.hpp"
#include "Util.hpp"
#include "rendering/RenderSettings.hpp"

#include <iterator>
#include <regex>
#include <set>
#include <stdexcept>

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

		std::vector<ShaderStageSourceFileInfo*> stageInfos;
		stageInfos.push_back(&sourceFileInfo.vertexInfo);

		if (sourceFileInfo.geometryInfo)
		{
			stageInfos.push_back(&*sourceFileInfo.geometryInfo);
		}

		if (sourceFileInfo.fragmentInfo)
		{
			stageInfos.push_back(&*sourceFileInfo.fragmentInfo);
		}

		std::vector<std::string> includeBuffer;

		for (auto* const stageInfo : stageInfos)
		{
			// Paths of files we already included in this stage's source.
			// We don't include them more than once.
			std::set<std::filesystem::path> includedFilePaths;

			bool doAnotherPass;

			do
			{
				doAnotherPass = false;

				for (auto lineIt = std::begin(stageInfo->fileContents); lineIt != std::end(stageInfo->fileContents); ++lineIt)
				{
					if (std::regex_search(*lineIt, includeLineRegex))
					{
						std::string includeFileName;

						if (std::regex_match(*lineIt, includeLineQuoteRegex))
						{
							std::regex_iterator regIt{std::begin(*lineIt), std::end(*lineIt), fileNameInQuotesRegex};
							includeFileName = regIt->str().substr(regIt->length() - 2);
						}
						else if (std::regex_match(*lineIt, includeLineBracketRegex))
						{
							std::regex_iterator regIt{std::begin(*lineIt), std::end(*lineIt), fileNameInBracketsRegex};
							includeFileName = regIt->str().substr(regIt->length() - 2);
						}
						else
						{
							auto const errMsg = "Error parsing shader includes: ill-formed file specifier was found.";
							internal::Logger::Instance().Error(errMsg);
							throw std::runtime_error{errMsg};
						}

						auto const includePath = absolute(stageInfo->absolutePath.parent_path() / includeFileName).make_preferred();

						lineIt->clear();

						if (includedFilePaths.contains(includePath))
						{
							continue;
						}

						includeBuffer.clear();
						ReadFileLines(includePath, includeBuffer);

						if (includeBuffer.empty())
						{
							continue;
						}

						lineIt->clear();
						
						stageInfo->fileContents.insert(lineIt, std::make_move_iterator(std::begin(includeBuffer)), std::make_move_iterator(std::end(includeBuffer)));

						doAnotherPass = true;
						break;
					}
				}
			}
			while (doAnotherPass);
		}

		ShaderProgramSourceInfo programSourceInfo;
		programSourceInfo.vertex = std::move(sourceFileInfo.vertexInfo.fileContents);

		if (sourceFileInfo.geometryInfo)
		{
			programSourceInfo.geometry = std::move(sourceFileInfo.geometryInfo->fileContents);
		}

		if (sourceFileInfo.fragmentInfo)
		{
			programSourceInfo.fragment = std::move(sourceFileInfo.fragmentInfo->fileContents);
		}

		return programSourceInfo;
	}
}
