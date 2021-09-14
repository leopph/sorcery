#include "Shader.hpp"

#include "../config/Settings.hpp"

#include "../util/logger.h"

#include <glad/glad.h>

#include <fstream>
#include <memory>
#include <vector>


namespace leopph::impl
{
	Shader::Shader(const Type type) :
		id{m_ID}, m_ID{}, m_ProgramFileName{GetFileName(type)}
	{
		const char* vertexSource{nullptr};
		const char* fragmentSource{nullptr};

		switch (type)
		{
			case Type::OBJECT:
				vertexSource = s_VertexSource.c_str();
				fragmentSource = s_FragmentSource.c_str();
				Logger::Instance().Debug("Constructing general shader.");
				break;

			case Type::SKYBOX:
				vertexSource = s_SkyboxVertexSource.c_str();
				fragmentSource = s_SkyboxFragmentSource.c_str();
				Logger::Instance().Debug("Constructing skybox shader.");
				break;

			case Type::DIRECTIONAL_SHADOW_MAP:
				vertexSource = s_DirectionalShadowMapVertexSource.c_str();
				fragmentSource = s_DirectionalShadowMapFragmentSource.c_str();
				Logger::Instance().Debug("Constructing directional shadow map shader.");
				break;

			case Type::GPASS_OBJECT:
				vertexSource = s_GPassObjectVertexSource.c_str();
				fragmentSource = s_GPassObjectFragmentSource.c_str();
				Logger::Instance().Debug("Constructing geometry pass object shader.");
				break;

			case Type::LIGHTPASS:
				vertexSource = s_LightPassVertexSource.c_str();
				fragmentSource = s_LightPassFragmentSource.c_str();
				Logger::Instance().Debug("Constructing lighting pass shader.");
				break;

			case Type::DIRLIGHTPASS:
				vertexSource = s_LightPassVertexSource.c_str();
				fragmentSource = s_DirLightPassFragmentSource.c_str();
				Logger::Instance().Debug("Constructing directional lighting pass shader.");
				break;

			case Type::TEXTURE:
				vertexSource = s_LightPassVertexSource.c_str();
				fragmentSource = s_TextureFragmentSource.c_str();
				Logger::Instance().Debug("Constructing full screen texture shader.");
				break;
		}

		if (Settings::IsCachingShaders())
		{
			Logger::Instance().Debug("Looking for shader caches.");

			const std::filesystem::path fullPath{Settings::ShaderCacheLocation() / m_ProgramFileName};

			if (std::filesystem::exists(fullPath))
			{
				Logger::Instance().Debug("Found cached shader. Attempting to parse.");

				if (ReadFromCache(fullPath))
				{
					Logger::Instance().Debug("Successfully parsed shader.");
					return;
				}

				Logger::Instance().Debug("Failed to parse shader from cache. Reverting to compilation.");
			}
			else
			{
				Logger::Instance().Debug("No cached shader found. Compiling.");
			}

			Compile(vertexSource, fragmentSource);

			Logger::Instance().Debug("Writing shader to cache.");
			WriteToCache(fullPath);
		}
		else
		{
			Logger::Instance().Debug("Compiling shader.");
			Compile(vertexSource, fragmentSource);
		}
	}


	void Shader::Compile(const char* vertexSource, const char* fragmentSource)
	{
		const unsigned vertexShaderID{glCreateShader(GL_VERTEX_SHADER)};
		const unsigned fragmentShaderID{glCreateShader(GL_FRAGMENT_SHADER)};

		glShaderSource(vertexShaderID, 1, &vertexSource, nullptr);
		glCompileShader(vertexShaderID);

		GLint status;
		glGetShaderiv(vertexShaderID, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetShaderiv(vertexShaderID, GL_INFO_LOG_LENGTH, &logLength);
			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(vertexShaderID, logLength, &logLength, errMsg.data());
			Logger::Instance().Error(errMsg);
		}


		glShaderSource(fragmentShaderID, 1, &fragmentSource, nullptr);
		glCompileShader(fragmentShaderID);

		glGetShaderiv(fragmentShaderID, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetShaderiv(fragmentShaderID, GL_INFO_LOG_LENGTH, &logLength);
			std::string errMsg;
			errMsg.resize(logLength);
			glGetShaderInfoLog(fragmentShaderID, logLength, &logLength, errMsg.data());
			Logger::Instance().Error(errMsg);
		}


		m_ID = glCreateProgram();
		glAttachShader(m_ID, vertexShaderID);
		glAttachShader(m_ID, fragmentShaderID);
		glLinkProgram(m_ID);

		glGetProgramiv(m_ID, GL_LINK_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLength;
			glGetProgramiv(m_ID, GL_INFO_LOG_LENGTH, &logLength);
			std::string errMsg;
			errMsg.resize(logLength);
			glGetProgramInfoLog(m_ID, logLength, &logLength, errMsg.data());
			Logger::Instance().Error(errMsg);
		}

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}


	bool Shader::ReadFromCache(const std::filesystem::path& path)
	{
		std::ifstream input{path, std::ios::binary};
		std::vector<char> buffer{std::istreambuf_iterator<char>(input), {}};
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

		auto binary{std::make_unique<char[]>(binaryLength)};
		GLenum binaryFormat;
		GLsizei actualLength;
		glGetProgramBinary(m_ID, static_cast<GLsizei>(binaryLength), &actualLength, &binaryFormat, binary.get());

		std::ofstream output{path, std::ios::binary};
		output.write(static_cast<const char*>(binary.get()), actualLength);
		output.close();
	}


	void Shader::Use() const
	{
		glUseProgram(m_ID);
	}


	void Shader::SetUniform(std::string_view name, bool value) const
	{
		glProgramUniform1i(m_ID, glGetUniformLocation(m_ID, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, int value) const
	{
		glProgramUniform1i(m_ID, glGetUniformLocation(m_ID, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, unsigned value) const
	{
		glProgramUniform1ui(m_ID, glGetUniformLocation(m_ID, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, float value) const
	{
		glProgramUniform1f(m_ID, glGetUniformLocation(m_ID, name.data()), value);
	}


	void Shader::SetUniform(std::string_view name, const Vector3& value) const
	{
		glProgramUniform3fv(m_ID, glGetUniformLocation(m_ID, name.data()), 1, value.Data());
	}


	void Shader::SetUniform(std::string_view name, const Matrix4& value) const
	{
		glProgramUniformMatrix4fv(m_ID, glGetUniformLocation(m_ID, name.data()), 1, GL_TRUE, value.Data());
	}


	void Shader::SetUniform(const std::string_view name, const std::vector<float>& value) const
	{
		glProgramUniform1fv(m_ID, glGetUniformLocation(m_ID, name.data()), static_cast<GLsizei>(value.size()), value.data());
	}


	void Shader::SetUniform(std::string_view name, const std::vector<Vector3>& value) const
	{
		glProgramUniform3fv(m_ID, glGetUniformLocation(m_ID, name.data()), value.size(), reinterpret_cast<const GLfloat*>(value.data()));
	}


	void Shader::SetUniform(std::string_view name, const std::vector<Matrix4>& value) const
	{
		glProgramUniformMatrix4fv(m_ID, glGetUniformLocation(m_ID, name.data()), value.size(), GL_TRUE, reinterpret_cast<const GLfloat*>(value.data()));
	}


	Shader::~Shader()
	{
		glDeleteProgram(m_ID);
	}


	std::string Shader::GetFileName(Type type)
	{
		switch (type)
		{
			case Type::OBJECT:
				return "objectShader";

			case Type::SKYBOX:
				return "skyboxShader";

			case Type::DIRECTIONAL_SHADOW_MAP:
				return "directionalShadowMapShader";

			case Type::GPASS_OBJECT:
				return "gpassObjectShader";

			case Type::LIGHTPASS:
				return "lightPassShader";

			case Type::DIRLIGHTPASS:
				return "dirLightPassShader";

			case Type::TEXTURE:
				return "textureShader";
		}

		return "";
	}
}