#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStageInfo.hpp"
#include "ShaderType.hpp"
#include "../../util/equal/StringEqual.hpp"
#include "../../util/hash/StringHash.hpp"

#include <map>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <vector>


namespace leopph::impl
{
	class ShaderFamily
	{
		public:
			explicit ShaderFamily(const std::vector<ShaderStageInfo>& stages);

			ShaderFamily(const ShaderFamily& other) = delete;
			ShaderFamily& operator=(const ShaderFamily& other) = delete;

			ShaderFamily(ShaderFamily&& other) = delete;
			ShaderFamily& operator=(ShaderFamily&& other) = delete;

			~ShaderFamily() = default;

			void SetBufferBinding(std::string_view bufName, int bindingIndex);

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

			static const std::string AmbLightFragSrc;
			static const std::string DirLightPassFragSrc;
			static const std::string SpotLightPassFragSrc;
			static const std::string PointLightPassFragSrc;

		private:
			class FlagInfo final
			{
				public:
					explicit FlagInfo(const std::unordered_set<std::string>& flags);

					[[nodiscard]]
					bool Empty() const;
					void Clear();

					bool& operator[](const std::string& flag);
					const bool& operator[](const std::string& flag) const;

					explicit operator std::vector<bool>() const;
					explicit operator std::vector<std::string>() const;

				private:
					std::map<std::string, bool> m_Flags;
			};


			class FlagInfoProxy final
			{
				public:
					explicit FlagInfoProxy(FlagInfo flagInfo);

					[[nodiscard]]
					bool Empty() const;
					void Clear();

					bool& operator[](const std::string& flag);
					const bool& operator[](const std::string& flag) const;

					explicit operator std::vector<bool>() const;
					explicit operator std::vector<std::string>() const;

				private:
					FlagInfo m_FlagInfo;
			};


		public:
			[[nodiscard]]
			FlagInfoProxy GetFlagInfo() const;
			[[nodiscard]]
			ShaderProgram& GetPermutation(const FlagInfoProxy& flagInfo);

		private:
			struct ProcessedSource
			{
				std::unordered_set<std::string> flags;
				std::vector<std::string> srcLines;
			};


			[[nodiscard]]
			static std::string BuildSourceString(std::vector<std::string> srcLines, const std::vector<std::string>& flags);
			static ProcessedSource ProcessSource(const std::string& src);

			std::unordered_map<std::string, int, StringHash, StringEqual> m_Bindings;
			std::unordered_map<std::vector<bool>, ShaderProgram> m_Permutations;
			std::unordered_map<ShaderType, std::vector<std::string>> m_Sources;
			std::unordered_set<std::string> m_Flags;
	};
}
