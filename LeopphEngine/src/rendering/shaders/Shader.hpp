#pragma once

#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>



namespace leopph::impl
{
	class Shader
	{
		public:
			Shader(const Shader& other) = delete;
			Shader(Shader&& other) = delete;

			void operator=(const Shader& other) = delete;
			void operator=(Shader&& other) = delete;

			virtual ~Shader();

			const unsigned& Id;

			void Use() const;

			void SetUniform(std::string_view name, bool value) const;
			void SetUniform(std::string_view name, int value) const;
			void SetUniform(std::string_view name, unsigned value) const;
			void SetUniform(std::string_view name, float value) const;
			void SetUniform(std::string_view name, const Vector3& value) const;
			void SetUniform(std::string_view name, const Matrix4& value) const;
			void SetUniform(std::string_view name, const std::vector<float>& value) const;
			void SetUniform(std::string_view name, const std::vector<Vector3>& value) const;
			void SetUniform(std::string_view name, const std::vector<Matrix4>& value) const;


		protected:
			static const std::string s_ObjectVertSrc;
			static const std::string s_ObjectFragSrc;
			static const std::string s_SkyboxVertSrc;
			static const std::string s_SkyboxFragSrc;
			static const std::string s_DirShadowMapVertSrc;
			static const std::string s_DirShadowMapFragSrc;
			static const std::string s_GPassObjectVertSrc;
			static const std::string s_GPassObjectFragSrc;
			static const std::string s_LightPassVertSrc;
			static const std::string s_LightPassFragSrc;
			static const std::string s_DirLightPassFragSrc;
			static const std::string s_TextureFragSrc;

			Shader(std::string_view vertSrc, std::string_view fragSrc);


		private:
			const unsigned m_ProgramName;
			const std::filesystem::path m_ProgramFileName;

			void Compile(std::string_view vertSrc, std::string_view fragSrc) const;

			bool ReadFromCache(const std::filesystem::path& path);
			void WriteToCache(const std::filesystem::path& path);

			// True for error-free
			static bool CheckForCompilationErrors(unsigned shaderName);
			// True for error-free
			static bool CheckForLinkErrors(unsigned programName);
	};
}
