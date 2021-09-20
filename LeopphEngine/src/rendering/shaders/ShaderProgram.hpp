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
			void SetUniform(std::string_view name, const std::vector<float>& value);
			void SetUniform(std::string_view name, const std::vector<Vector3>& value);
			void SetUniform(std::string_view name, const std::vector<Matrix4>& value);

			const unsigned& Name;


		protected:
			static const std::string s_ObjectVertSrc;
			static const std::string s_ObjectFragSrc;
			static const std::string s_SkyboxVertSrc;
			static const std::string s_SkyboxFragSrc;
			static const std::string s_ShadowMapVertSrc;
			static const std::string s_GPassObjectVertSrc;
			static const std::string s_GPassObjectFragSrc;
			static const std::string s_LightPassVertSrc;
			static const std::string s_LightPassFragSrc;
			static const std::string s_DirLightPassFragSrc;
			static const std::string s_TextureFragSrc;
			static const std::string s_AmbLightFragSrc;
			static const std::string s_SpotLightPassFragSrc;

			explicit ShaderProgram(const std::vector<ShaderStage>& stages);


		private:
			[[nodiscard]] std::optional<std::string> CheckForLinkErrors() const;
			[[nodiscard]] unsigned GetUniformLocation(std::string_view);

			std::unordered_map<std::string_view, unsigned> m_UniformLocations;
			unsigned m_Name;
	};
}