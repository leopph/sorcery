#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStageInfo.hpp"
#include "ShaderType.hpp"
#include "../../util/equal/StringEqual.hpp"
#include "../../util/hash/StringHash.hpp"
#include "../../util/less/StringLess.hpp"

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
			explicit ShaderFamily(const std::vector<ShaderStageInfo>& stages);

			ShaderFamily(const ShaderFamily& other) = default;
			auto operator=(const ShaderFamily& other) -> ShaderFamily& = default;

			ShaderFamily(ShaderFamily&& other) = delete;
			auto operator=(ShaderFamily&& other) -> ShaderFamily& = delete;

			~ShaderFamily() noexcept = default;

			auto SetBufferBinding(std::string_view bufName, int bindingIndex) -> void;

			// Uses the currently set flags to look up or generate a permutation.
			[[nodiscard]] auto GetPermutation() -> ShaderProgram&;

			// Cleans all currently set flags.
			auto Clear() -> void;

			[[nodiscard]] auto operator[](std::string_view key) -> std::string&;

			static const std::string ObjectVertSrc;
			static const std::string ObjectFragSrc;

			static const std::string SkyboxVertSrc;
			static const std::string SkyboxFragSrc;

			static const std::string ShadowMapVertSrc;
			static const std::string CubeShadowMapVertSrc;
			static const std::string CubeShadowMapGeomSrc;
			static const std::string CubeShadowMapFragSrc;

			static const std::string GPassObjectVertSrc;
			static const std::string GPassObjectFragSrc;

			static const std::string LightPassVertSrc;
			static const std::string LightPassFragSrc;

			static const std::string AmbLightFragSrc;
			static const std::string DirLightPassFragSrc;
			static const std::string SpotLightPassFragSrc;
			static const std::string PointLightPassFragSrc;

		private:
			// Create a source that has the currently set flags inserted.
			[[nodiscard]] auto BuildSrcString(std::string_view src) const -> std::string;
			// Create the permutation key from the currently set flags
			[[nodiscard]] auto BuildPermString() const -> std::string;

			std::unordered_map<std::string, int, StringHash, StringEqual> m_Bindings;
			std::unordered_map<ShaderType, std::string> m_Sources;

			// The flags currently set by a consumer. This will be used to generate a permutation.
			std::map<std::string, std::string, StringLess> m_CurrentFlags;
			// The permutations with key being the in format {name1:value1;name2:value2}.
			std::unordered_map<std::string, ShaderProgram> m_Permutations;
	};
}
