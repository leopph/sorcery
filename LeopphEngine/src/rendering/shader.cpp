#include "shader.h"

#include "../config/settings.h"

#include <fstream>
#include <glad/glad.h>
#include <iostream>
#include <memory>
#include <vector>

namespace leopph::impl
{
	const std::filesystem::path Shader::s_ProgramFileName{ "shader" };


	Shader::Shader()
	{
		if (Settings::IsCachingShaders())
		{
			std::filesystem::path fullPath{ Settings::ShaderCacheLocation() / s_ProgramFileName };

			if (std::filesystem::exists(fullPath) && ReadFromCache(fullPath))
					return;

			Compile();
			WriteToCache(fullPath);
		}
		else
		{
			Compile();
		}
	}


	void Shader::Compile()
	{
		const char* vertexSource{ s_VertexSource.c_str() };
		const char* fragmentSource{ s_FragmentSource.c_str() };


		unsigned vertexShaderID{ glCreateShader(GL_VERTEX_SHADER) };
		unsigned fragmentShaderID{ glCreateShader(GL_FRAGMENT_SHADER) };

		glShaderSource(vertexShaderID, 1, &vertexSource, nullptr);
		glCompileShader(vertexShaderID);

		int status;
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during vertex shader compilation!" << std::endl;


		glShaderSource(fragmentShaderID, 1, &fragmentSource, nullptr);
		glCompileShader(fragmentShaderID);

		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during fragment shader compilation!" << std::endl;


		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertexShaderID);
		glAttachShader(m_ID, fragmentShaderID);
		glLinkProgram(m_ID);

		glGetProgramiv(m_ID, GL_LINK_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during shader program linking!" << std::endl;

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}

	bool Shader::ReadFromCache(const std::filesystem::path& path)
	{
		std::ifstream input{ path, std::ios::binary };
		std::vector<char> buffer{ std::istreambuf_iterator<char>(input), {} };
		input.close();

		GLint nFormats;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &nFormats);

		std::vector<int> formats(nFormats);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &formats[0]);

		m_ID = glCreateProgram();

		GLint status;

		for (int format : formats)
		{
			glProgramBinary(m_ID, static_cast<GLenum>(format), buffer.data(), static_cast<GLsizei>(buffer.size()));

			if (glGetProgramiv(m_ID, GL_LINK_STATUS, &status); status == GL_TRUE)
				return true;
		}

		return false;
	}

	void Shader::WriteToCache(const std::filesystem::path& path)
	{
		GLint binaryLength;
		glGetProgramiv(m_ID, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

		auto binary{ std::make_unique<char[]>(binaryLength) };
		GLenum binaryFormat;
		GLsizei actualLength;
		glGetProgramBinary(m_ID, static_cast<GLsizei>(binaryLength), &actualLength, &binaryFormat, binary.get());

		std::ofstream output{ path, std::ios::binary };
		output.write(static_cast<const char*>(binary.get()), actualLength);
		output.close();
	}


	unsigned Shader::GetID() const { return m_ID; }

	void Shader::Use() const { glUseProgram(m_ID); }

	void Shader::SetUniform(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, const Vector3& value) const { glUniform3fv(glGetUniformLocation(m_ID, name.data()), 1, value.Data()); }
	void Shader::SetUniform(const std::string& name, const Matrix4& value) const { glUniformMatrix4fv(glGetUniformLocation(m_ID, name.data()), 1, GL_TRUE, value.Data()); }

	Shader::~Shader()
	{
		glDeleteProgram(m_ID);
	}
}