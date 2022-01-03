#include "ShaderFamily.hpp"

#include <algorithm>
#include <iterator>
#include <utility>


namespace leopph::internal
{
	ShaderFamily::ShaderFamily(const std::vector<ShaderStageInfo>& stages)
	{
		std::ranges::transform(stages, std::inserter(m_Sources, m_Sources.begin()), [](const auto& stageInfo)
		{
			return std::make_pair(stageInfo.Type, stageInfo.Src);
		});
	}


	auto ShaderFamily::SetBufferBinding(const std::string_view bufName, const int bindingIndex) -> void
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


	auto ShaderFamily::GetPermutation() -> ShaderProgram&
	{
		// Get the permutation id string
		auto permStr{BuildPermString()};

		// Look up existing permutation
		if (const auto it{m_Permutations.find(permStr)}; it != m_Permutations.end())
		{
			return it->second;
		}

		// Not found

		std::vector<ShaderStageInfo> stageInfos;
		stageInfos.reserve(m_Sources.size());

		// Create new permutation
		for (const auto& [type, src] : m_Sources)
		{
			stageInfos.emplace_back(BuildSrcString(src), type);
		}

		auto& shaderProgram{m_Permutations.emplace(std::move(permStr), stageInfos).first->second};

		// Set up bindings for it
		for (const auto& [bufName, binding] : m_Bindings)
		{
			shaderProgram.SetBufferBinding(bufName, binding);
		}

		return shaderProgram;
	}


	auto ShaderFamily::Clear() -> void
	{
		m_CurrentFlags.clear();
	}


	auto ShaderFamily::operator[](const std::string_view key) -> std::string&
	{
		if (const auto it{m_CurrentFlags.find(key)}; it != m_CurrentFlags.end())
		{
			return it->second;
		}
		return m_CurrentFlags.emplace(std::string{key}, "").first->second;
	}


	auto ShaderFamily::BuildSrcString(const std::string_view src) const -> std::string
	{
		std::string ret{src};
		for (const auto& [name, value] : m_CurrentFlags)
		{
			ret.insert(ret.find_first_of('\n') + 1, std::string{"#define "}.append(name).append(1, ' ').append(value).append(1, '\n'));
		}
		return ret;
	}


	auto ShaderFamily::BuildPermString() const -> std::string
	{
		std::string ret{"{"};
		for (const auto& [name, value] : m_CurrentFlags)
		{
			ret.append(name).append(1, ':').append(value).append(1, ';');
		}
		ret.replace(ret.length() - 1, 1, "");
		return ret;
	}
}
