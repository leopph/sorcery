#include "ShaderFamily.hpp"

#include "../../util/logger.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>
#include <utility>


namespace leopph::internal
{
	ShaderFamily::ShaderFamily(const std::vector<ShaderStageInfo>& stages)
	{
		std::ranges::for_each(stages, [this](const auto& stageInfo)
		{
			auto processedSrc{ProcessSource(stageInfo.src)};
			m_Sources[stageInfo.type] = std::move(processedSrc.srcLines);
			m_Flags.merge(processedSrc.flags);
		});
	}

	void ShaderFamily::SetBufferBinding(const std::string_view bufName, const int bindingIndex)
	{
		if (const auto it{m_Bindings.find(bufName)};
			it != m_Bindings.end())
		{
			it->second = bindingIndex;
		}
		else
		{
			m_Bindings.emplace(bufName, bindingIndex);
		}
		for (auto& [bitmap, shaderProgram] : m_Permutations)
		{
			shaderProgram.SetBufferBinding(bufName, bindingIndex);
		}
	}

	ShaderFamily::FlagInfoProxy ShaderFamily::GetFlagInfo() const
	{
		return FlagInfoProxy{FlagInfo{m_Flags}};
	}

	ShaderProgram& ShaderFamily::GetPermutation(const FlagInfoProxy& flagInfo)
	{
		const auto flagBitMap{static_cast<std::vector<bool>>(flagInfo)};

		if (const auto it{m_Permutations.find(flagBitMap)};
			it != m_Permutations.end())
		{
			return it->second;
		}

		std::vector<ShaderStageInfo> stageInfos;
		const auto flagList{static_cast<std::vector<std::string>>(flagInfo)};

		std::ranges::for_each(m_Sources, [&](const auto& srcPair)
		{
			stageInfos.emplace_back(BuildSourceString(srcPair.second, flagList), srcPair.first);
		});

		auto& shaderProgram{m_Permutations.emplace(flagBitMap, stageInfos).first->second};
		for (const auto& [bufName, binding] : m_Bindings)
		{
			shaderProgram.SetBufferBinding(bufName, binding);
		}
		return shaderProgram;
	}

	std::string ShaderFamily::BuildSourceString(std::vector<std::string> srcLines, const std::vector<std::string>& flags)
	{
		std::string ret;

		if (srcLines.empty())
		{
			Logger::Instance().Error("List of shader source lines was empty.");
			return ret;
		}

		for (std::size_t i = 0; i < flags.size(); ++i)
		{
			srcLines.emplace(srcLines.begin() + 1 + static_cast<long long>(i), "#define " + flags[i]);
		}

		std::ranges::for_each(srcLines, [&](auto& line)
		{
			ret += std::move(line) + '\n';
		});

		return ret;
	}

	ShaderFamily::ProcessedSource ShaderFamily::ProcessSource(const std::string& src)
	{
		ProcessedSource ret;

		std::istringstream inStream{src};

		for (std::string line; std::getline(inStream, line);)
		{
			if (!line.empty() && !(line.find_first_not_of(' ') == std::string::npos) && !(line.find_first_not_of('\t') == std::string::npos))
			{
				for (constexpr std::array directives{"#ifdef ", "#ifndef "};
				     const auto& directive : directives)
				{
					const auto directiveLength{std::strlen(directive)};

					if (const auto index{line.find(directive)};
						index != std::string::npos)
					{
						ret.flags.insert(line.substr(index + directiveLength, line.length() - index - directiveLength + 1));
						break;
					}
				}
				ret.srcLines.push_back(std::move(line));
			}
		}
		return ret;
	}
}
