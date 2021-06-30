#include "shader.h"

#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <iostream>

namespace leopph::impl
{
	Shader::Shader()
	{
		const char* vertexSource{ s_VertexSource.c_str() };
		const char* fragmentSource{ s_FragmentSource.c_str() };


		// compile vertex shader
		unsigned vertexShaderID{ glCreateShader(GL_VERTEX_SHADER) };
		glShaderSource(vertexShaderID, 1, &vertexSource, nullptr);
		glCompileShader(vertexShaderID);

		int status;
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during vertex shader compilation!" << std::endl;


		// compile fragment shader
		unsigned fragmentShaderID{ glCreateShader(GL_FRAGMENT_SHADER) };
		glShaderSource(fragmentShaderID, 1, &fragmentSource, nullptr);
		glCompileShader(fragmentShaderID);

		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during fragment shader compilation!" << std::endl;


		// create shader program
		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertexShaderID);
		glAttachShader(m_ID, fragmentShaderID);
		glLinkProgram(m_ID);

		glGetProgramiv(m_ID, GL_LINK_STATUS, &status);
		if (!status)
			std::cerr << "There was an error during shader program linking!" << std::endl;


		// cleanup
		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}



	// SHADER ID GETTER
	unsigned Shader::GetID() const { return m_ID; }



	// BIND SHADER PROGRAM
	void Shader::Use() const { glUseProgram(m_ID); }



	// UNIFORM SETTER OVERLOADS
	void Shader::SetUniform(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(m_ID, name.data()), value); }
	void Shader::SetUniform(const std::string& name, const Vector3& value) const { glUniform3fv(glGetUniformLocation(m_ID, name.data()), 1, value.Data()); }
	void Shader::SetUniform(const std::string& name, const Matrix4& value) const { glUniformMatrix4fv(glGetUniformLocation(m_ID, name.data()), 1, GL_TRUE, value.Data()); }



	// DESTRUCTOR
	Shader::~Shader()
	{
		glDeleteProgram(m_ID);
	}
}