#pragma once

#include "vector.h"
#include "matrix.h"

#include <filesystem>
#include <string>

namespace leopph
{
	/* A CLASS THAT REPRESENTS AN OPENGL SHADER PROGRAM
	CURRENTLY INCORPORATES A VERTEX AND A FRAGMENT SHADER */
	class Shader
	{
	private:
		unsigned m_ID;

	public:
		Shader(const std::filesystem::path& vertexPath, const std::filesystem::path& fragmentPath);
		~Shader();

		unsigned GetID() const;

		void Use() const;

		void SetUniform(const std::string& name, bool value) const;
		void SetUniform(const std::string& name, int value) const;
		void SetUniform(const std::string& name, float value) const;
		void SetUniform(const std::string& name, const Vector3& value) const;
		void SetUniform(const std::string& name, const Matrix4& value) const;
	};
}