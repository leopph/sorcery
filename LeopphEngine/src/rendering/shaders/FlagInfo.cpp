#include "ShaderFamily.hpp"

#include <algorithm>


namespace leopph::impl
{
	ShaderFamily::FlagInfo::FlagInfo(const std::unordered_set<std::string>& flags)
	{
		std::ranges::for_each(flags, [&](auto& flag)
		{
			m_Flags.emplace(std::move(flag), false);
		});
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

	bool& ShaderFamily::FlagInfo::operator[](const std::string& flag)
	{
		return m_Flags.at(flag);
	}

	const bool& ShaderFamily::FlagInfo::operator[](const std::string& flag) const
	{
		return m_Flags.at(flag);
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

	ShaderFamily::FlagInfoProxy::FlagInfoProxy(FlagInfo flagInfo) :
		m_FlagInfo{std::move(flagInfo)}
	{}

	bool ShaderFamily::FlagInfoProxy::Empty() const
	{
		return m_FlagInfo.Empty();
	}

	void ShaderFamily::FlagInfoProxy::Clear()
	{
		m_FlagInfo.Clear();
	}

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
}
