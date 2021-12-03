#pragma once

#include "ShaderProgram.hpp"
#include "ShaderStageInfo.hpp"
#include "ShaderType.hpp"

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
			ShaderFamily(ShaderFamily&& other) = delete;

			~ShaderFamily();

			ShaderFamily& operator=(const ShaderFamily& other) = delete;
			ShaderFamily& operator=(ShaderFamily&& other) = delete;

			static const std::string ObjectVertSrc;
			static const std::string ObjectVertInstancedSrc;
			static const std::string ObjectFragSrc;

			static const std::string SkyboxVertSrc;
			static const std::string SkyboxFragSrc;

			static const std::string ShadowMapVertSrc;
			static const std::string ShadowMapVertInstancedSrc;
			static const std::string CubeShadowMapVertSrc;
			static const std::string CubeShadowMapVertInstancedSrc;
			static const std::string CubeShadowMapGeomSrc;
			static const std::string CubeShadowMapFragSrc;

			static const std::string GPassObjectVertSrc;
			static const std::string GPassObjectVertInstancedSrc;
			static const std::string GPassObjectFragSrc;

			static const std::string LightPassVertSrc;

			static const std::string AmbLightFragSrc;
			static const std::string DirLightPassFragSrc;
			static const std::string SpotLightPassFragSrc;
			static const std::string PointLightPassFragSrc;


		private:
			class FlagInfoBase
			{
				public:
					FlagInfoBase() = default;
					FlagInfoBase(const FlagInfoBase& other) = default;
					FlagInfoBase(FlagInfoBase&& other) = default;

					virtual ~FlagInfoBase() = default;

					FlagInfoBase& operator=(const FlagInfoBase& other) = default;
					FlagInfoBase& operator=(FlagInfoBase&& other) = default;

					virtual bool& operator[](const std::string& flag) = 0;
					virtual const bool& operator[](const std::string& flag) const = 0;
					virtual explicit operator std::vector<bool>() const = 0;
					virtual explicit operator std::vector<std::string>() const = 0;

					[[nodiscard]] virtual bool Empty() const = 0;

					virtual void Clear() = 0;
			};


		class FlagInfo final : FlagInfoBase
		{
			public:
				explicit FlagInfo(const std::unordered_set<std::string>& flags);
				FlagInfo(const FlagInfo&) = default;
				FlagInfo(FlagInfo&& other) noexcept;

				FlagInfo& operator=(const FlagInfo& other) = default;
				FlagInfo& operator=(FlagInfo&& other) noexcept;

				~FlagInfo() override = default;


				bool& operator[](const std::string& flag) override;
				const bool& operator[](const std::string& flag) const override;
				explicit operator std::vector<bool>() const override;
				explicit operator std::vector<std::string>() const override;

				[[nodiscard]] bool Empty() const override;

				void Clear() override;


			private:
				std::map<std::string, bool> m_Flags;
		};


		class FlagInfoProxy final : public FlagInfoBase
		{
			public:
				explicit FlagInfoProxy(FlagInfo flagInfo);


				bool& operator[](const std::string& flag) override;
				const bool& operator[](const std::string& flag) const override;
				explicit operator std::vector<bool>() const override;
				explicit operator std::vector<std::string>() const override;

				[[nodiscard]] bool Empty() const override;

				void Clear() override;


			private:
				FlagInfo m_FlagInfo;
		};


		public:
			[[nodiscard]] FlagInfoProxy GetFlagInfo() const;
			[[nodiscard]] ShaderProgram& GetPermutation(const FlagInfoProxy& flagInfo);


		private:
			struct ProcessedSource
			{
				std::unordered_set<std::string> flags;
				std::vector<std::string> srcLines;
			};
			
			[[nodiscard]] static std::string BuildSourceString(std::vector<std::string> srcLines, const std::vector<std::string>& flags);
			static ProcessedSource ProcessSource(const std::string& src);

			std::unordered_map<std::vector<bool>, ShaderProgram> m_Permutations;
			std::unordered_map<ShaderType, std::vector<std::string>> m_Sources;
			std::unordered_set<std::string> m_Flags;
		};
}
