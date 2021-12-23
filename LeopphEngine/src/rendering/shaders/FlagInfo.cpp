#include "ShaderFamily.hpp"

#include <algorithm>


namespace leopph::internal
{
	ShaderFamily::FlagInfo::FlagInfo(const std::unordered_set<std::string>& flags)
	{
		std::ranges::for_each(flags, [&](auto& flag)
		{
			m_Flags.emplace(std::move(flag), false);
		});
	}

	auto ShaderFamily::FlagInfo::Empty() const -> bool
	{
		return m_Flags.empty();
	}

	auto ShaderFamily::FlagInfo::Clear() -> void
	{
		std::ranges::for_each(m_Flags, [](auto& flagPair)
		{
			flagPair.second = false;
		});
	}

	auto ShaderFamily::FlagInfo::operator[](const std::string& flag) -> bool&
	{
		return m_Flags.at(flag);
	}

	auto ShaderFamily::FlagInfo::operator[](const std::string& flag) const -> const bool&
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

	auto ShaderFamily::FlagInfoProxy::Empty() const -> bool
	{
		return m_FlagInfo.Empty();
	}

	auto ShaderFamily::FlagInfoProxy::Clear() -> void
	{
		m_FlagInfo.Clear();
	}

	auto ShaderFamily::FlagInfoProxy::operator[](const std::string& flag) -> bool&
	{
		return m_FlagInfo[flag];
	}

	auto ShaderFamily::FlagInfoProxy::operator[](const std::string& flag) const -> const bool&
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
