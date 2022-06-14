#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStageInfo.hpp"
#include "ShaderType.hpp"
#include "../../util/equal/StringEqual.hpp"
#include "../../util/hash/StringHash.hpp"
#include "../../util/less/StringLess.hpp"

#include <cstddef>
#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>


namespace leopph::internal
{
	// A set of shaders with different flags set using the same source code.
	class ShaderFamily
	{
		public:
			explicit ShaderFamily(std::vector<ShaderStageInfo> const& stages);

			ShaderFamily(ShaderFamily const& other) = default;
			auto operator=(ShaderFamily const& other) -> ShaderFamily& = default;

			ShaderFamily(ShaderFamily&& other) = delete;
			auto operator=(ShaderFamily&& other) -> ShaderFamily& = delete;

			~ShaderFamily() noexcept = default;

			auto SetBufferBinding(std::string_view bufName, int bindingIndex) -> void;

			// Uses the currently set flags to look up or generate a permutation.
			[[nodiscard]] auto GetPermutation() -> ShaderProgram&;

			// Cleans all currently set flags.
			auto Clear() -> void;

			[[nodiscard]] auto operator[](std::string_view key) -> std::string&;

			static std::string const ObjectVertSrc;
			static std::string const ObjectFragSrc;

			static std::string const SkyboxVertSrc;
			static std::string const SkyboxFragSrc;

			static std::string const DepthShadowVertSrc;

			static std::string const LinearShadowVertSrc;
			static std::string const LinearShadowFragSrc;

			static std::string const GeometryPassVertSrc;
			static std::string const GeometryPassFragSrc;

			static std::string const LightPassVertSrc;
			static std::string const LightPassFragSrc;

			static std::string const TranspCompositeVertSrc;
			static std::string const TranspCompositeFragSrc;

		private:
			// Create a source that has the currently set flags inserted.
			[[nodiscard]] auto BuildSrcString(std::string_view src) const -> std::string;
			// Create the permutation key from the currently set flags
			[[nodiscard]] auto BuildPermString() const -> std::string;
			// Sets the current buffer bindings in the passed shader
			auto SetBufferBinding(ShaderProgram& shader) -> void;
			// Builds the shader using the current flags, sets its buffer bindings, stores it, then returns it
			auto BuildFromSources(std::string permStr) -> ShaderProgram&;
			// Returns a list of the hashes of the shader sources in Vertex->Geom->Fragment order
			[[nodiscard]] auto SourceHashes() const -> std::vector<std::size_t> const&;

			std::unordered_map<std::string, int, StringHash, StringEqual> m_Bindings;
			std::unordered_map<ShaderType, std::string> m_Sources;

			// The flags currently set by a consumer. This will be used to generate a permutation.
			std::map<std::string, std::string, StringLess> m_CurrentFlags;
			// The permutations with key being the in format {name1:value1;name2:value2}.
			std::unordered_map<std::string, ShaderProgram> m_Permutations;

			// Cache for the hashes of the source strings
			mutable std::vector<std::size_t> m_SrcHashCache;
			mutable bool m_SrcHashCacheRdy{false};
			constexpr static char const* CACHE_PREFIX{"shad_"};
	};
}
