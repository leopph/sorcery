#include "rendering/shaders/ShaderCommon.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "Util.hpp"

#include <regex>
#include <sstream>

namespace leopph
{
	auto ResolveShaderIncludes(ShaderStageSourceFileInfo const& fileInfo) -> std::string
	{
		auto changed = true;
		auto processedSrc = fileInfo.source;

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
							internal::Logger::Instance().Error("Incomplete preprocessor directive on line " + std::to_string(i) + '.');
							return {};
						}

						view.remove_prefix(numWhiteSpace);

						if (view.starts_with("include"))
						{
							view.remove_prefix(7);

							numWhiteSpace = view.find_first_not_of(whiteSpaceChars);

							if (numWhiteSpace == std::string_view::npos)
							{
								internal::Logger::Instance().Error("Incomplete include directive on line " + std::to_string(i) + ": missing file specifier.");
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
								internal::Logger::Instance().Error("Ill-formed include directive on line " + std::to_string(i) + ": unknown token in place of file specifier.");
								return {};
							}

							view.remove_prefix(1);

							auto const filenameLngth = view.find_first_of(fileSpecClosingChar);

							if (filenameLngth == std::string_view::npos)
							{
								internal::Logger::Instance().Error("Ill-formed include directive on line " + std::to_string(i) + ": missing file specifier closer.");
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


	auto ExtractShaderOptions(std::string& source, std::unordered_map<std::string, ShaderOption>& out) -> u32
	{
		std::regex const static validFullOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s+min\s*=\s*\d+\s+max\s*=\s*\d+\s*$)delim"};
		std::regex const static validShortOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s*$)delim"};
		std::regex const static nameAssignment{R"delim(=\s*[A-Za-z][\S]*)delim"};
		std::regex const static numberAssignment{R"delim(=\s*\d+)delim"};
		std::regex const static nameIdentifier{R"delim([A-Za-z][\S]*)delim"};
		std::regex const static number{R"delim(\d+)delim"};

		u32 nextFreeId{0};

		std::vector<std::string> lines;
		SplitLines(source, lines);

		std::ostringstream outStream;

		for (auto& line : lines)
		{
			if (std::regex_match(line, validFullOptionRegex))
			{
				std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
				auto const assignmentStr = nameAssignmentMatch->str();
				std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};
				auto const name = nameMatch->str();

				std::regex_iterator numberAssignmentMatch{std::begin(line), std::end(line), numberAssignment};
				auto const minAssignmentStr = numberAssignmentMatch->str();
				std::regex_iterator minMatch{std::begin(minAssignmentStr), std::end(minAssignmentStr), number};
				u32 const min = std::stoi(minMatch->str());

				++numberAssignmentMatch;
				auto const maxAssignmentStr = numberAssignmentMatch->str();
				std::regex_iterator maxMatch{std::begin(maxAssignmentStr), std::end(maxAssignmentStr), number};
				u32 const max = std::stoi(maxMatch->str());

				ShaderOption option;
				option.id = nextFreeId;
				option.min = min;
				option.max = max;

				out[name] = option;

				nextFreeId += math::BinaryDigitCount(max - min);
			}
			else if (std::regex_match(line, validShortOptionRegex))
			{
				std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
				auto const assignmentStr = nameAssignmentMatch->str();
				std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};
				auto const name = nameMatch->str();

				ShaderOption option;
				option.id = nextFreeId;
				option.min = 0;
				option.max = 1;

				out[name] = option;

				nextFreeId += 1;
			}
			else
			{
				outStream << line << '\n';
			}
		}

		source = outStream.str();
		return nextFreeId;
	}
}
