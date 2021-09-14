#include "Shader.hpp"

#include "../../config/Settings.hpp"
#include "../../util/logger.h"

#include <glad/glad.h>

#include <fstream>
#include <memory>
#include <vector>



namespace leopph::impl
{
	Shader::Shader(const std::string_view vertSrc, const std::string_view fragSrc) :
		Id{m_ProgramName}, m_ProgramName{glCreateProgram()}
	{
		if (Settings::IsCachingShaders())
		{
			const auto fullPath{Settings::ShaderCacheLocation() / m_ProgramFileName};

			if (exists(fullPath) && ReadFromCache(fullPath))
			{
				return;
			}

			Compile(vertSrc, fragSrc);
			WriteToCache(fullPath);
		}
		else
		{
			Compile(vertSrc, fragSrc);
		}
	}


	Shader::~Shader()
	{
		glDeleteProgram(m_ProgramName);
	}


	void Shader::Compile(const std::string_view vertSrc, const std::string_view fragSrc) const
	{
		if (!vertSrc.empty())
		{
			const auto vertName{glCreateShader(GL_VERTEX_SHADER)};
			glShaderSource(vertName, 1, std::array{vertSrc.data()}.data(), nullptr);
			glCompileShader(vertName);

			if (CheckForCompilationErrors(vertName))
			{
				glAttachShader(m_ProgramName, vertName);
			}

			glDeleteShader(vertName);
		}

		if (!fragSrc.empty())
		{
			const auto fragName{glCreateShader(GL_FRAGMENT_SHADER)};
			glShaderSource(fragName, 1, std::array{fragSrc.data()}.data(), nullptr);
			glCompileShader(fragName);

			if (CheckForCompilationErrors(fragName))
			{
				glAttachShader(m_ProgramName, fragName);
			}

			glDeleteShader(fragName);
		}

		glLinkProgram(m_ProgramName);

		if (!CheckForLinkErrors(m_ProgramName))
		{
			// TODO handle error
		}

	}


	bool Shader::ReadFromCache(const std::filesystem::path& path) const
	{
		std::ifstream input{path, std::ios::binary};
		const std::vector buffer{std::istreambuf_iterator(input), {}};
		input.close();

		GLint nFormats;
		glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &nFormats);

		std::vector<int> formats(nFormats);
		glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, &formats[0]);

		GLint status;

		for (auto format : formats)
		{
			glProgramBinary(m_ProgramName, static_cast<GLenum>(format), buffer.data(), static_cast<GLsizei>(buffer.size()));

			if (glGetProgramiv(m_ProgramName, GL_LINK_STATUS, &status); status == GL_TRUE)
			{
				return true;
			}
		}

		return false;
	}


	void Shader::WriteToCache(const std::filesystem::path& path) const
	{
		GLint binaryLength;
		glGetProgramiv(m_ProgramName, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

		auto binary{std::make_unique<char[]>(binaryLength)};
		GLenum binaryFormat;
		GLsizei actualLength;
		glGetProgramBinary(m_ProgramName, static_cast<GLsizei>(binaryLength), &actualLength, &binaryFormat, binary.get());

		std::ofstream output{path, std::ios::binary};
		output.write(static_cast<const char*>(binary.get()), actualLength);
		output.close();
	}


	void Shader::Use() const
	{
		glUseProgram(m_ProgramName);
	}


	void Shader::SetUniform(std::string_view name, bool value) const
	{
		glProgramUniform1i(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, int value) const
	{
		glProgramUniform1i(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, unsigned value) const
	{
		glProgramUniform1ui(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, float value) const
	{
		glProgramUniform1f(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, const Vector3& value) const
	{
		glProgramUniform3fv(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), 1, value.Data());
	}


	void Shader::SetUniform(std::string_view name, const Matrix4& value) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), 1, GL_TRUE, value.Data());
	}


	void Shader::SetUniform(const std::string_view name, const std::vector<float>& value) const
	{
		glProgramUniform1fv(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), static_cast<GLsizei>(value.size()), value.data());
	}


	void Shader::SetUniform(std::string_view name, const std::vector<Vector3>& value) const
	{
		glProgramUniform3fv(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), static_cast<GLsizei>(value.size()), reinterpret_cast<const GLfloat*>(value.data()));
	}


	void Shader::SetUniform(std::string_view name, const std::vector<Matrix4>& value) const
	{
		glProgramUniformMatrix4fv(m_ProgramName, glGetUniformLocation(m_ProgramName, name.data()), static_cast<GLsizei>(value.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(value.data()));
	}


	bool Shader::CheckForCompilationErrors(const unsigned shaderName)
	{
		GLint status;
		glGetShaderiv(shaderName, GL_COMPILE_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetShaderiv(shaderName, GL_INFO_LOG_LENGTH, &logLength);

			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(shaderName, logLength, &logLength, errMsg.data());

			Logger::Instance().Error(errMsg);
		}

		return status == GL_TRUE;
	}


	bool Shader::CheckForLinkErrors(const unsigned programName)
	{
		GLint status;
		glGetProgramiv(programName, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetProgramiv(programName, GL_INFO_LOG_LENGTH, &logLength);

			std::string errMsg;
			errMsg.resize(logLength);
			glGetProgramInfoLog(programName, logLength, &logLength, errMsg.data());

			Logger::Instance().Error(errMsg);
		}

		return status == GL_TRUE;
	}

}
