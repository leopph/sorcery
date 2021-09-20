#include "ShaderProgram.hpp"

#include "../../util/logger.h"

#include <glad/glad.h>

#include <algorithm>



namespace leopph::impl
{
	ShaderProgram::ShaderProgram(const std::vector<ShaderStageInfo>& stages) :
		Name{m_Name},
		m_Name{glCreateProgram()}
	{
		std::ranges::for_each(stages, [this](const auto& stageInfo)
		{
			ShaderStage{stageInfo}.AttachTo(*this);
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


	void ShaderProgram::SetUniform(const std::string_view name, const bool value)
	{
		glProgramUniform1i(m_Name, GetUniformLocation(name), value);
	}


	void ShaderProgram::SetUniform(const std::string_view name, const int value)
	{
		glProgramUniform1i(m_Name, GetUniformLocation(name), value);
	}


	void ShaderProgram::SetUniform(const std::string_view name, const unsigned value)
	{
		glProgramUniform1ui(m_Name, GetUniformLocation(name), value);
	}


	void ShaderProgram::SetUniform(const std::string_view name, const float value)
	{
		glProgramUniform1f(m_Name, GetUniformLocation(name), value);
	}


	void ShaderProgram::SetUniform(const std::string_view name, const Vector3& value)
	{
		glProgramUniform3fv(m_Name, GetUniformLocation(name), 1, value.Data().data());
	}


	void ShaderProgram::SetUniform(const std::string_view name, const Matrix4& value)
	{
		glProgramUniformMatrix4fv(m_Name, GetUniformLocation(name), 1, GL_TRUE, reinterpret_cast<const GLfloat*>(value.Data().data()));
	}


	void ShaderProgram::SetUniform(std::string_view name, const std::vector<bool>& value)
	{
		// TODO
	}


	void ShaderProgram::SetUniform(std::string_view name, const std::vector<int>& value)
	{
		glProgramUniform1iv(m_Name, GetUniformLocation(name), value.size(), value.data());
	}


	void ShaderProgram::SetUniform(std::string_view name, const std::vector<unsigned>& value)
	{
		glProgramUniform1uiv(m_Name, GetUniformLocation(name), value.size(), value.data());
	}


	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<float>& value)
	{
		glProgramUniform1fv(m_Name, GetUniformLocation(name), static_cast<GLsizei>(value.size()), value.data());
	}


	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<Vector3>& value)
	{
		glProgramUniform3fv(m_Name, GetUniformLocation(name), static_cast<GLsizei>(value.size()), reinterpret_cast<const GLfloat*>(value.data()));
	}


	void ShaderProgram::SetUniform(const std::string_view name, const std::vector<Matrix4>& value)
	{
		glProgramUniformMatrix4fv(m_Name, GetUniformLocation(name), static_cast<GLsizei>(value.size()), GL_TRUE, reinterpret_cast<const GLfloat*>(value.data()));
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


	int ShaderProgram::GetUniformLocation(const std::string_view name)
	{
		auto it{m_UniformLocations.find(name)};

		if (it == m_UniformLocations.end())
		{
			it = m_UniformLocations.emplace(name, glGetUniformLocation(m_Name, name.data())).first;
		}

		return it->second;
	}
}
