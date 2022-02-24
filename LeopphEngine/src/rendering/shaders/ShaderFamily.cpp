#include "ShaderFamily.hpp"

#include "../../config/Settings.hpp"
#include "../../util/logger.h"

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <functional>
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

		// If caching
		if (const auto cacheLoc{Settings::ShaderCacheLocation()};
			!cacheLoc.empty())
		{
			const auto permHash{decltype(m_Permutations)::hasher()(permStr)};
			const auto cachePath{[&, this]
			{
				std::filesystem::path p{CACHE_PREFIX + std::to_string(permHash)};
				for (const auto hash : SourceHashes())
				{
					p += "_" + std::to_string(hash); 
				}
				return cacheLoc / p;
			}()};

			// Try parsing cache
			if (std::filesystem::exists(cachePath))
			{
				std::ifstream input(cachePath, std::ios::binary);
				std::vector<unsigned char> binary{std::istreambuf_iterator(input), {}};
				const auto [it, inserted]{m_Permutations.emplace(permStr, binary)};
				if (inserted)
				{
					Logger::Instance().Debug("Shader parsed from " + cachePath.string() + ".");
					SetBufferBinding(it->second);
					return it->second;
				}
			}

			// Build and cache it
			auto& shader{BuildFromSources(std::move(permStr))};
			const auto binary{shader.Binary()};
			std::ofstream output(cachePath, std::ios::binary);
			std::ranges::copy(binary, std::ostreambuf_iterator(output));
			Logger::Instance().Debug("Shader cached to " + cachePath.string() + ".");
			return shader;
		}

		return BuildFromSources(std::move(permStr));		
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


	auto ShaderFamily::SetBufferBinding(ShaderProgram& shader) -> void
	{
		for (const auto& [bufName, binding] : m_Bindings)
		{
			shader.SetBufferBinding(bufName, binding);
		}
	}


	auto ShaderFamily::BuildFromSources(std::string permStr) -> ShaderProgram&
	{
		std::vector<ShaderStageInfo> stageInfos;
		stageInfos.reserve(m_Sources.size());

		// Create new permutation
		for (const auto& [type, src] : m_Sources)
		{
			stageInfos.emplace_back(BuildSrcString(src), type);
		}

		auto& shaderProgram{m_Permutations.emplace(std::move(permStr), stageInfos).first->second};

		// Set up bindings for it
		SetBufferBinding(shaderProgram);

		return shaderProgram;
	}


	auto ShaderFamily::SourceHashes() const -> const std::vector<std::size_t>&
	{
		if (m_SrcHashCacheRdy)
		{
			return m_SrcHashCache;
		}

		std::vector<ShaderType> types;
		types.reserve(m_Sources.size());
		for (const auto& [type, src] : m_Sources)
		{
			types.push_back(type);
		}
		std::ranges::sort(types);
		
		for (constexpr std::hash<std::string> hasher; const auto type : types)
		{
			m_SrcHashCache.push_back(hasher(m_Sources.at(type)));
		}
		m_SrcHashCacheRdy = true;
		return m_SrcHashCache;
	}
}
