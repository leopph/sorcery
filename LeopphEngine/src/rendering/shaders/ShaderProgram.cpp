#include "ShaderProgram.hpp"

#include "../../util/logger.h"
#include "../../util/interface/OpenGLAdapter.hpp"

#include <glad/glad.h>


namespace leopph::internal
{
	ShaderProgram::ShaderProgram(const std::vector<ShaderStageInfo>& stageInfo) :
		m_ProgramName{glCreateProgram()}
	{
		std::vector<unsigned> shaderNames;

		std::ranges::for_each(stageInfo, [&](const auto& info)
		{
			const auto shaderName{glCreateShader(OpenGLAdapter::OpenGLShaderType(info.type))};
			glShaderSource(shaderName, 1, std::array{info.src.data()}.data(), nullptr);
			glCompileShader(shaderName);

			if (const auto status{CompilationStatus(shaderName)};
				status.second.has_value())
			{
				if (status.first)
				{
					Logger::Instance().Debug(status.second.value());
				}
				else
				{
					Logger::Instance().Error(status.second.value());
					glDeleteShader(shaderName);
					return;
				}
			}

			shaderNames.push_back(shaderName);
			glAttachShader(m_ProgramName, shaderName);
		});

		glLinkProgram(m_ProgramName);

		if (const auto status{LinkStatus()};
			status.second.has_value())
		{
			if (status.first)
			{
				Logger::Instance().Debug(status.second.value());
			}
			else
			{
				Logger::Instance().Error(status.second.value());
			}
		}

		std::ranges::for_each(shaderNames, [](const auto& shaderName)
		{
			glDeleteShader(shaderName);
		});
	}

	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(m_ProgramName);
	}

	void ShaderProgram::Use() const
	{
		glUseProgram(m_ProgramName);
	}

	void ShaderProgram::Unuse() const
	{
		glUseProgram(0);
	}

	void ShaderProgram::SetUniform(const std::string_view name, const bool value)
	{
		glProgramUniform1i(m_ProgramName, GetUniformLocation(name), value);
	}

	void ShaderProgram::SetUniform(const std::string_view name, const int value)
	{
		glProgramUniform1i(m_ProgramName, GetUniformLocation(name), value);
	}

	void ShaderProgram::SetUniform(const std::string_view name, const unsigned value)
	{
		glProgramUniform1ui(m_ProgramName, GetUniformLocation(name), value);
	}

	void ShaderProgram::SetUniform(const std::string_view name, const float value)
	{
		glProgramUniform1f(m_ProgramName, GetUniformLocation(name), value);
	}

	void ShaderProgram::SetUniform(const std::string_view name, const Vector3& value)
	{
		glProgramUniform3fv(m_ProgramName, GetUniformLocation(name), 1, value.Data().data());
	}

	void ShaderProgram::SetUniform(const std::string_view name, const Matrix4& value)
	{
		glProgramUniformMatrix4fv(m_ProgramName, GetUniformLocation(name), 1, GL_TRUE, reinterpret_cast<const GLfloat*>(value.Data().data()));
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<bool>& value)
	{
		// TODO
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<int>& value)
	{
		glProgramUniform1iv(m_ProgramName, GetUniformLocation(name), static_cast<GLsizei>(value.size()), value.data());
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<unsigned>& value)
	{
		glProgramUniform1uiv(m_ProgramName, GetUniformLocation(name), static_cast<GLsizei>(value.size()), value.data());
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<float>& value)
	{
		glProgramUniform1fv(m_ProgramName, GetUniformLocation(name), static_cast<GLsizei>(value.size()), value.data());
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<Vector3>& value)
	{
		glProgramUniform3fv(m_ProgramName, GetUniformLocation(name), static_cast<GLsizei>(value.size()), reinterpret_cast<const GLfloat*>(value.data()));
	}

	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<Matrix4>& value)
	{
		glProgramUniformMatrix4fv(m_ProgramName, GetUniformLocation(name), static_cast<GLsizei>(value.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(value.data()));
	}

	void ShaderProgram::SetBufferBinding(const std::string_view bufName, const int bindingIndex)
	{
		
	}

	std::pair<bool, std::optional<std::string>> ShaderProgram::CompilationStatus(const unsigned name)
	{
		std::pair<bool, std::optional<std::string>> ret;

		GLint logLength;
		glGetShaderiv(name, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0)
		{
			std::string infoLog;
			infoLog.resize(logLength);
			glGetShaderInfoLog(name, logLength, &logLength, infoLog.data());
			ret.second = infoLog;
		}

		GLint status;
		glGetShaderiv(name, GL_COMPILE_STATUS, &status);
		ret.first = status == GL_TRUE;
		return ret;
	}

	std::pair<bool, std::optional<std::string>> ShaderProgram::LinkStatus() const
	{
		std::pair<bool, std::optional<std::string>> ret;

		GLint logLength;
		glGetProgramiv(m_ProgramName, GL_INFO_LOG_LENGTH, &logLength);

		if (logLength > 0)
		{
			std::string infoLog;
			infoLog.resize(logLength);
			glGetProgramInfoLog(m_ProgramName, logLength, &logLength, infoLog.data());
			ret.second = infoLog;
		}

		GLint status;
		glGetProgramiv(m_ProgramName, GL_LINK_STATUS, &status);
		ret.first = status == GL_TRUE;
		return ret;
	}

	int ShaderProgram::GetUniformLocation(const std::string_view name)
	{
		auto it{m_UniformLocations.find(name)};

		if (it == m_UniformLocations.end())
		{
			it = m_UniformLocations.emplace(name, glGetUniformLocation(m_ProgramName, name.data())).first;
		}

		return it->second;
	}


}
