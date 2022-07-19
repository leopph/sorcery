#include "rendering/shaders/ShaderFamily.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "Util.hpp"
#include "rendering/shaders/ShaderProcessing.hpp"

#include <iterator>
#include <optional>
#include <ranges>
#include <regex>
#include <sstream>
#include <stdexcept>

namespace leopph
{
	ShaderFamily::ShaderFamily(ShaderProgramSourceInfo const& sourceInfo) :
		m_VertexSource{sourceInfo.vertex},
		m_GeometrySource{sourceInfo.geometry},
		m_FragmentSource{sourceInfo.fragment}
	{
		auto bitsRequired = ExtractOptions(m_VertexSource, m_Options);

		if (m_GeometrySource)
		{
			bitsRequired = ExtractOptions(*m_GeometrySource, m_Options);
		}

		if (m_FragmentSource)
		{
			bitsRequired = ExtractOptions(*m_FragmentSource, m_Options);
		}

		std::vector<std::vector<bool>> permutationBits;
		permutationBits.emplace_back(bitsRequired, false);

		std::vector<std::vector<bool>> tmpBuffer;

		for (auto const& [min, max, id] : m_Options | std::views::values)
		{
			tmpBuffer.clear();

			for (auto const& bitset : permutationBits)
			{
				auto const range = max - min;

				for (u32 i = 1; i <= range; i++)
				{
					auto newBitset = bitset;
					auto const digits = math::BinaryDigitCount(i);

					for (u8 j = 0; j < digits; j++)
					{
						newBitset[id + j] = static_cast<bool>(i & (0x00000001 << j));
						// TODO this only generates the permutations bitsets without the defines. we need to associate the defines with the bitsets
					}

					tmpBuffer.emplace_back(std::move(newBitset));
				}
			}

			permutationBits.insert(std::end(permutationBits), std::make_move_iterator(std::begin(tmpBuffer)), std::make_move_iterator(std::end(tmpBuffer)));
		}
	}

	auto ShaderFamily::Option(std::string_view const name, u32 const value) -> void
	{
		auto const& option = m_Options.find(name)->second;

		#ifndef NDEBUG
		if (option.min > value || option.max < value)
		{
			auto const errMsg = "Value [" + std::to_string(value) + "] was out of range for shader option [" + std::string{name} + "].";
			internal::Logger::Instance().Error(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif

		auto const digits = math::BinaryDigitCount(value);

		for (auto i = 0; i < digits; i++)
		{
			m_OptionBits[option.id + i] = static_cast<bool>(value & (0x00000001 << i));
		}
	}

	auto ShaderFamily::ExtractOptions(std::vector<std::string>& sourceLines, std::unordered_map<std::string, ShaderOption, StringHash, StringEqual>& out) -> u32
	{
		std::regex const static validFullOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s+min\s*=\s*\d+\s+max\s*=\s*\d+\s*$)delim"};
		std::regex const static validShortOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s*$)delim"};
		std::regex const static nameAssignment{R"delim(=\s*[A-Za-z][\S]*)delim"};
		std::regex const static numberAssignment{R"delim(=\s*\d+)delim"};
		std::regex const static nameIdentifier{R"delim([A-Za-z][\S]*)delim"};
		std::regex const static number{R"delim(\d+)delim"};

		u32 nextFreeId{0};

		std::vector<std::string> lines;
		SplitLines(sourceLines, lines);

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

				ShaderOption option
				{
					.min = min,
					.max = max,
					.id = nextFreeId
				};

				if (out.contains(name))
				{
					if (out[name].min != option.min && out[name].max != option.max)
					{
						auto const errMsg = "Duplicate shader option [" + name + "] found while parsing options.";
						internal::Logger::Instance().Error(errMsg);
						throw std::runtime_error{errMsg};
					}
				}
				else
				{
					out[name] = option;
					nextFreeId += math::BinaryDigitCount(max - min);
				}
			}
			else if (std::regex_match(line, validShortOptionRegex))
			{
				std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
				auto const assignmentStr = nameAssignmentMatch->str();
				std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};
				auto const name = nameMatch->str();

				ShaderOption option
				{
					.min = 0,
					.max = 1,
					.id = nextFreeId
				};

				if (out.contains(name))
				{
					if (out[name].min != option.min && out[name].max != option.max)
					{
						auto const errMsg = "Duplicate shader option [" + name + "] found while parsing options.";
						internal::Logger::Instance().Error(errMsg);
						throw std::runtime_error{errMsg};
					}
				}
				else
				{
					out[name] = option;
					nextFreeId += 1;
				}
			}
			else
			{
				outStream << line << '\n';
			}
		}

		sourceLines = outStream.str();
		return nextFreeId;
	}

	auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath) -> ShaderFamily
	{
		auto fileInfo = ReadShaderFiles(std::move(vertexShaderPath), std::move(geometryShaderPath), std::move(fragmentShaderPath));
		auto sourceInfo = ProcessShaderIncludes(std::move(fileInfo));
		InsertBuiltInShaderLines(sourceInfo);
		return ShaderFamily{sourceInfo};
	}
}
