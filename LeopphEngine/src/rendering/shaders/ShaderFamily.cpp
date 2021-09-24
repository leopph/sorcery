#include "ShaderFamily.hpp"

#include "../../util/logger.h"

#include <algorithm>
#include <array>
#include <cstring>
#include <sstream>
#include <stdexcept>
#include <utility>



namespace leopph::impl
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


	ShaderFamily::~ShaderFamily() = default;


	ShaderFamily::FlagInfo::FlagInfo(const std::unordered_set<std::string>& flags)
	{
		std::ranges::for_each(flags, [&](auto& flag)
		{
			m_Flags.emplace(std::move(flag), false);
		});
	}


	ShaderFamily::FlagInfo::FlagInfo(FlagInfo&& other) noexcept :
		m_Flags{std::move(other.m_Flags)}
	{}


	ShaderFamily::FlagInfo& ShaderFamily::FlagInfo::operator=(FlagInfo&& other) noexcept
	{
		m_Flags = std::move(other.m_Flags);
		return *this;
	}



	bool& ShaderFamily::FlagInfo::operator[](const std::string& flag)
	{
		return const_cast<bool&>(const_cast<const FlagInfo*>(this)->operator[](flag));
	}


	const bool& ShaderFamily::FlagInfo::operator[](const std::string& flag) const
	{
		if (const auto& it{m_Flags.find(flag)};
			it != m_Flags.end())
		{
			return it->second;
		}

		throw std::runtime_error{"Invalid flag."};
	}


	ShaderFamily::FlagInfo::operator std::vector<bool>() const
	{
		std::vector<bool> ret;

		std::ranges::for_each(m_Flags, [&](const auto& flagPair)
		{
			ret.push_back(flagPair.second);
		});

		return ret;
	}


	ShaderFamily::FlagInfo::operator std::vector<std::string>() const
	{
		std::vector<std::string> ret;

		std::ranges::for_each(m_Flags, [&](const auto& flagPair)
		{
			if (flagPair.second)
			{
				ret.push_back(flagPair.first);
			}
		});

		return ret;
	}



	bool ShaderFamily::FlagInfo::Empty() const
	{
		return m_Flags.empty();
	}


	void ShaderFamily::FlagInfo::Clear()
	{
		std::ranges::for_each(m_Flags, [](auto& flagPair)
		{
			flagPair.second = false;
		});
	}


	ShaderFamily::FlagInfoProxy::FlagInfoProxy(FlagInfo flagInfo) :
		m_FlagInfo{std::move(flagInfo)}
	{}


	bool& ShaderFamily::FlagInfoProxy::operator[](const std::string& flag)
	{
		return m_FlagInfo[flag];
	}


	const bool& ShaderFamily::FlagInfoProxy::operator[](const std::string& flag) const
	{
		return m_FlagInfo[flag];
	}


	ShaderFamily::FlagInfoProxy::operator std::vector<bool>() const
	{
		return static_cast<std::vector<bool>>(m_FlagInfo);
	}


	ShaderFamily::FlagInfoProxy::operator std::vector<std::string>() const
	{
		return static_cast<std::vector<std::string>>(m_FlagInfo);
	}


	bool ShaderFamily::FlagInfoProxy::Empty() const
	{
		return m_FlagInfo.Empty();
	}


	void ShaderFamily::FlagInfoProxy::Clear()
	{
		m_FlagInfo.Clear();
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

		return m_Permutations.emplace(flagBitMap, stageInfos).first->second;
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
