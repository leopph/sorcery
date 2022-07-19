#include "rendering/shaders/ShaderProcessing.hpp"

#include "Logger.hpp"
#include "Util.hpp"
#include "rendering/RenderSettings.hpp"

#include <iterator>
#include <regex>
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

		ShaderProgramSourceInfo programSourceInfo;

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

	auto InsertBuiltInShaderLines(ShaderProgramSourceInfo& sourceInfo) -> void
	{
		std::vector<std::string> builtInLines;
		builtInLines.emplace_back("#version 450 core\n");
		builtInLines.emplace_back("#pragma option name=DIR_SHADOW\n");
		builtInLines.push_back(std::string{"#pragma option name=NUM_DIR_CASCADES min="}.append(std::to_string(rendersettings::numShadowCascades)).append(" max=").append(std::to_string(rendersettings::numShadowCascades)).append(1, '\n'));
		builtInLines.push_back(std::string{"#pragma option name=NUM_SPOT min=0 max="}.append(std::to_string(rendersettings::numMaxSpot)).append(1, '\n'));
		builtInLines.push_back(std::string{"#pragma option name=NUM_SPOT_SHADOW min=0 max="}.append(std::to_string(rendersettings::numMaxSpotShadow)).append(1, '\n'));
		builtInLines.push_back(std::string{"#pragma option name=NUM_POINT min=0 max="}.append(std::to_string(rendersettings::numMaxPoint)).append(1, '\n'));
		builtInLines.push_back(std::string{"#pragma option name=NUM_POINT_SHADOW min=0 max="}.append(std::to_string(rendersettings::numMaxPointShadow)).append(1, '\n'));

		sourceInfo.vertex.insert(std::begin(sourceInfo.vertex), std::begin(builtInLines), std::end(builtInLines));

		if (sourceInfo.geometry)
		{
			sourceInfo.geometry->insert(std::begin(*sourceInfo.geometry), std::begin(builtInLines), std::end(builtInLines));
		}

		if (sourceInfo.fragment)
		{
			sourceInfo.fragment->insert(std::begin(*sourceInfo.fragment), std::begin(builtInLines), std::end(builtInLines));
		}
	}
}
