#pragma once

#include "../math/Vector.hpp"
#include "../math/Matrix.hpp"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace leopph::impl
{
	class Shader
	{
	public:
		enum class Type
		{
			OBJECT, SKYBOX, DIRECTIONAL_SHADOW_MAP, GPASS_OBJECT, LIGHTPASS, DIRLIGHTPASS, TEXTURE
		};

		explicit Shader(Type type);
		~Shader();

		const unsigned& id;

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

	private:
		unsigned m_ID;

		static std::string s_ObjectVertSource;
		static std::string s_ObjectFragSource;
		static std::string s_SkyboxVertSource;
		static std::string s_SkyboxFragSource;
		static std::string s_DirShadowMapVertSource;
		static std::string s_DirShadowMapFragSource;
		static std::string s_GPassObjectVertSource;
		static std::string s_GPassObjectFragSource;
		static std::string s_LightPassVertSource;
		static std::string s_LightPassFragSource;
		static std::string s_DirLightPassFragSource;
		static std::string s_TextureFragSource;

		const std::filesystem::path m_ProgramFileName;

		void Compile(const char* vertexSource, const char* fragmentSource);
		bool ReadFromCache(const std::filesystem::path& path);
		void WriteToCache(const std::filesystem::path& path);

		static std::string GetFileName(Type type);
	};
}