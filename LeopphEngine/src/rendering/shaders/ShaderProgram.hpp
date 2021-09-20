#pragma once

#include "ShaderStage.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>



namespace leopph::impl
{
	class ShaderProgram
	{
		public:
			explicit ShaderProgram(const std::vector<ShaderStageInfo>& stages);

			ShaderProgram(const ShaderProgram& other) = delete;
			ShaderProgram(ShaderProgram&& other) = delete;

			~ShaderProgram();

			ShaderProgram& operator=(const ShaderProgram& other) = delete;
			ShaderProgram& operator=(ShaderProgram&& other) = delete;

			void Use() const;
			void Unuse() const;

			void SetUniform(std::string_view name, bool value);
			void SetUniform(std::string_view name, int value);
			void SetUniform(std::string_view name, unsigned value);
			void SetUniform(std::string_view name, float value);
			void SetUniform(std::string_view name, const Vector3& value);
			void SetUniform(std::string_view name, const Matrix4& value);
			void SetUniform(std::string_view name, const std::vector<bool>& value);
			void SetUniform(std::string_view name, const std::vector<int>& value);
			void SetUniform(std::string_view name, const std::vector<unsigned>& value);
			void SetUniform(std::string_view name, const std::vector<float>& value);
			void SetUniform(std::string_view name, const std::vector<Vector3>& value);
			void SetUniform(std::string_view name, const std::vector<Matrix4>& value);

			const unsigned& Name;

			static const std::string ObjectVertSrc;
			static const std::string ObjectFragSrc;
			static const std::string SkyboxVertSrc;
			static const std::string SkyboxFragSrc;
			static const std::string ShadowMapVertSrc;
			static const std::string GPassObjectVertSrc;
			static const std::string GPassObjectFragSrc;
			static const std::string LightPassVertSrc;
			static const std::string LightPassFragSrc;
			static const std::string DirLightPassFragSrc;
			static const std::string TextureFragSrc;
			static const std::string AmbLightFragSrc;
			static const std::string SpotLightPassFragSrc;


		private:
			[[nodiscard]] std::optional<std::string> CheckForLinkErrors() const;
			[[nodiscard]] int GetUniformLocation(std::string_view);

			std::unordered_map<std::string_view, int> m_UniformLocations;
			unsigned m_Name;
	};
}
