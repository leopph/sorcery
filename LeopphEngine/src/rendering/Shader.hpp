#pragma once

#include "../math/Vector.hpp"
#include "../math/Matrix.hpp"

#include <filesystem>
#include <string>

namespace leopph::impl
{
	class Shader
	{
	public:
		enum class Type
		{
			OBJECT, SKYBOX, DIRECTIONAL_SHADOW_MAP, DEBUG
		};

		explicit Shader(Type type);
		~Shader();

		const unsigned& id;

		void Use() const;

		void SetUniform(const std::string& name, bool value) const;
		void SetUniform(const std::string& name, int value) const;
		void SetUniform(const std::string& name, float value) const;
		void SetUniform(const std::string& name, const Vector3& value) const;
		void SetUniform(const std::string& name, const Matrix4& value) const;

	private:
		unsigned m_ID;

		static std::string s_VertexSource;
		static std::string s_FragmentSource;
		static std::string s_SkyboxVertexSource;
		static std::string s_SkyboxFragmentSource;
		static std::string s_DirectionalShadowMapVertexSource;
		static std::string s_DirectionalShadowMapFragmentSource;
		static std::string s_DebugVertexSource;
		static std::string s_DebugFragmentSource;

		const std::filesystem::path m_ProgramFileName;

		void Compile(const char* vertexSource, const char* fragmentSource);
		bool ReadFromCache(const std::filesystem::path& path);
		void WriteToCache(const std::filesystem::path& path);

		static std::string GetFileName(Type type);
	};
}