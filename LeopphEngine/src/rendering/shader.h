#pragma once

#include "../math/vector.h"
#include "../math/matrix.h"

#include <filesystem>
#include <string>

namespace leopph::impl
{
	/* A CLASS THAT REPRESENTS AN OPENGL SHADER PROGRAM
	CURRENTLY INCORPORATES A VERTEX AND A FRAGMENT SHADER */
	class Shader
	{
	public:
		Shader();
		~Shader();

		unsigned GetID() const;

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
	};
}