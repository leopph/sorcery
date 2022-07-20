#include "rendering/shaders/ShaderProcessing.hpp"

#include "Logger.hpp"
#include "Util.hpp"
#include "rendering/RenderSettings.hpp"

#include <format>
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

						auto const GetFileNameFromLine = [](std::string_view const line, std::regex const& nameRegex) -> std::string
						{
							std::regex_iterator const regIt{std::begin(line), std::end(line), nameRegex};
							return regIt->str().substr(1, regIt->length() - 2);
						};

						if (std::regex_match(*lineIt, includeLineQuoteRegex))
						{
							includeFileName = GetFileNameFromLine(*lineIt, fileNameInQuotesRegex);
						}
						else if (std::regex_match(*lineIt, includeLineBracketRegex))
						{
							includeFileName = GetFileNameFromLine(*lineIt, fileNameInBracketsRegex);
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



	auto ExtractShaderOptions(ShaderProgramSourceInfo& sourceInfo) -> std::vector<ShaderOptionInfo>
	{
		std::regex const static validFullOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s+min\s*=\s*\d+\s+max\s*=\s*\d+\s*)delim"};
		std::regex const static validShortOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s*)delim"};
		std::regex const static nameAssignment{R"delim(=\s*[A-Za-z][\S]*)delim"};
		std::regex const static numberAssignment{R"delim(=\s*\d+)delim"};
		std::regex const static nameIdentifier{R"delim([A-Za-z][\S]*)delim"};
		std::regex const static number{R"delim(\d+)delim"};

		std::vector<ShaderOptionInfo> ret;

		auto const ExtractFrom = [&ret](std::vector<std::string>& lines) -> void
		{
			for (auto& line : lines)
			{
				if (std::regex_search(line, validFullOptionRegex))
				{
					std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
					auto const assignmentStr = nameAssignmentMatch->str();
					std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};

					std::regex_iterator numberAssignmentMatch{std::begin(line), std::end(line), numberAssignment};
					auto const minAssignmentStr = numberAssignmentMatch->str();
					std::regex_iterator minMatch{std::begin(minAssignmentStr), std::end(minAssignmentStr), number};

					++numberAssignmentMatch;
					auto const maxAssignmentStr = numberAssignmentMatch->str();
					std::regex_iterator maxMatch{std::begin(maxAssignmentStr), std::end(maxAssignmentStr), number};

					ret.emplace_back(nameMatch->str(), std::stoi(minMatch->str()), std::stoi(maxMatch->str()));
					line.clear();
				}
				else if (std::regex_search(line, validShortOptionRegex))
				{
					std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
					auto const assignmentStr = nameAssignmentMatch->str();
					std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};

					ret.emplace_back(nameMatch->str(), 0, 1);
					line.clear();
				}
			}
		};

		ExtractFrom(sourceInfo.vertex);

		if (sourceInfo.geometry)
		{
			ExtractFrom(*sourceInfo.geometry);
		}

		if (sourceInfo.fragment)
		{
			ExtractFrom(*sourceInfo.fragment);
		}

		for (std::size_t i = 0; i < ret.size(); i++)
		{
			for (auto j = i + 1; j < ret.size();)
			{
				if (ret[i].name == ret[j].name)
				{
					if (ret[i].minValue != ret[j].minValue || ret[i].maxValue != ret[j].maxValue)
					{
						auto const errMsg = std::format("Error extracting shader options: option [{}] was specified multiple times with different value sets.", ret[i].name);
						internal::Logger::Instance().Error(errMsg);
						throw std::runtime_error{errMsg};
					}

					ret.erase(std::begin(ret) + j);
				}
				else
				{
					j++;
				}
			}
		}

		return ret;
	}
}
