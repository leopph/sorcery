#include "ShaderProgram.hpp"

#include "../../util/logger.h"

#include <glad/glad.h>

#include <algorithm>


namespace leopph::impl
{
	ShaderProgram::ShaderProgram(const std::vector<ShaderStage>& stages) :
		Name{m_Name},
		m_Name{glCreateProgram()}
	{
		std::ranges::for_each(stages, [this](const auto& stage)
		{
			stage.AttachTo(*this);
		});


		glLinkProgram(m_Name);

		if (const auto errMsg{CheckForLinkErrors()};
			errMsg.has_value())
		{
			Logger::Instance().Error(errMsg.value());
			return;
		}
	}


	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(m_Name);
	}


	void ShaderProgram::Use() const
	{
		glUseProgram(m_Name);
	}


	void ShaderProgram::Unuse() const
	{
		glUseProgram(0);
	}


	void ShaderProgram::SetUniform(std::string_view name, bool value) const
	{
		glProgramUniform1i(m_Name, glGetUniformLocation(m_Name, name.data()), value);
	}


	void ShaderProgram::SetUniform(std::string_view name, int value) const
	{
		glProgramUniform1i(m_Name, glGetUniformLocation(m_Name, name.data()), value);
	}


	void ShaderProgram::SetUniform(std::string_view name, unsigned value) const
	{
		glProgramUniform1ui(m_Name, glGetUniformLocation(m_Name, name.data()), value);
	}


	void ShaderProgram::SetUniform(std::string_view name, float value) const
	{
		glProgramUniform1f(m_Name, glGetUniformLocation(m_Name, name.data()), value);
	}


	void ShaderProgram::SetUniform(std::string_view name, const Vector3& value) const
	{
		glProgramUniform3fv(m_Name, glGetUniformLocation(m_Name, name.data()), 1, value.Data().data());
	}


	void ShaderProgram::SetUniform(std::string_view name, const Matrix4& value) const
	{
		glProgramUniformMatrix4fv(m_Name, glGetUniformLocation(m_Name, name.data()), 1, GL_TRUE, reinterpret_cast<const GLfloat*>(value.Data().data()));
	}


	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<float>& value) const
	{
		glProgramUniform1fv(m_Name, glGetUniformLocation(m_Name, name.data()), static_cast<GLsizei>(value.size()), value.data());
	}


	void ShaderProgram::SetUniform(std::string_view name, const std::vector<Vector3>& value) const
	{
		glProgramUniform3fv(m_Name, glGetUniformLocation(m_Name, name.data()), static_cast<GLsizei>(value.size()), reinterpret_cast<const GLfloat*>(value.data()));
	}


	void ShaderProgram::SetUniform(std::string_view name, const std::vector<Matrix4>& value) const
	{
		glProgramUniformMatrix4fv(m_Name, glGetUniformLocation(m_Name, name.data()), static_cast<GLsizei>(value.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(value.data()));
	}


	std::optional<std::string> ShaderProgram::CheckForLinkErrors() const
	{
		GLint status;
		glGetProgramiv(m_Name, GL_LINK_STATUS, &status);

		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetProgramiv(m_Name, GL_INFO_LOG_LENGTH, &logLength);

			std::string errMsg;
			errMsg.resize(logLength);
			glGetProgramInfoLog(m_Name, logLength, &logLength, errMsg.data());
			return {errMsg};
		}

		return {};
	}
}