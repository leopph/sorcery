#include "rendering/shaders/ShaderProgram2.hpp"

#include "rendering/gl/GlCore.hpp"

namespace leopph
{
	ShaderProgram2::ShaderProgram2(ShaderProgramSourceInfo const& sourceInfo) :
		m_Program{glCreateProgram()}
	{
		if (sourceInfo.vertex)
		{
			m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
			auto const src = sourceInfo.vertex->data();
			GLint const lngth = sourceInfo.vertex->length();
			glShaderSource(m_VertexShader, 1, &src, &lngth);
			glCompileShader(m_VertexShader);
			glAttachShader(m_Program, m_VertexShader);
		}

		if (sourceInfo.geometry)
		{
			m_GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			auto const src = sourceInfo.geometry->data();
			GLint const lngth = sourceInfo.geometry->length();
			glShaderSource(m_GeometryShader, 1, &src, &lngth);
			glCompileShader(m_GeometryShader);
			glAttachShader(m_Program, m_GeometryShader);
		}

		if (sourceInfo.fragment)
		{
			m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			auto const src = sourceInfo.fragment->data();
			GLint const lngth = sourceInfo.fragment->length();
			glShaderSource(m_FragmentShader, 1, &src, &lngth);
			glCompileShader(m_FragmentShader);
			glAttachShader(m_Program, m_FragmentShader);
		}

		glLinkProgram(m_Program);
		QueryUniformLocations();
	}

	ShaderProgram2::ShaderProgram2(ShaderProgramBinaryInfo const& binaryInfo) :
		m_Program{glCreateProgram()}
	{
		if (binaryInfo.vertex)
		{
			m_VertexShader = glCreateShader(GL_VERTEX_SHADER);
			glShaderBinary(1, &m_VertexShader, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryInfo.vertex->binary.data(), binaryInfo.vertex->binary.size_bytes());
			glSpecializeShader(m_VertexShader, binaryInfo.vertex->entryPoint.data(), 0, nullptr, nullptr);
		}

		if (binaryInfo.geometry)
		{
			m_GeometryShader = glCreateShader(GL_GEOMETRY_SHADER);
			glShaderBinary(1, &m_GeometryShader, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryInfo.geometry->binary.data(), binaryInfo.geometry->binary.size_bytes());
			glSpecializeShader(m_GeometryShader, binaryInfo.geometry->entryPoint.data(), 0, nullptr, nullptr);
		}

		if (binaryInfo.fragment)
		{
			m_FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderBinary(1, &m_FragmentShader, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryInfo.fragment->binary.data(), binaryInfo.fragment->binary.size_bytes());
			glSpecializeShader(m_FragmentShader, binaryInfo.fragment->entryPoint.data(), 0, nullptr, nullptr);
		}

		glLinkProgram(m_Program);
		QueryUniformLocations();
	}

	ShaderProgram2::ShaderProgram2(ShaderProgramCachedBinaryInputInfo const& cachedBinaryInfo) :
		m_Program{glCreateProgram()}
	{
		glProgramBinary(m_Program, cachedBinaryInfo.format, cachedBinaryInfo.binary.data(), cachedBinaryInfo.binary.size_bytes());
		QueryUniformLocations();
	}

	auto ShaderProgram2::Uniform(std::string_view const name, bool const value) const -> void
	{
		glProgramUniform1i(m_Program, m_UniformLocations.find(name)->second, static_cast<GLint>(value));
	}

	auto ShaderProgram2::Uniform(std::string_view const name, i32 const value) const -> void
	{
		glProgramUniform1i(m_Program, m_UniformLocations.find(name)->second, value);
	}

	auto ShaderProgram2::Uniform(std::string_view const name, u32 const value) const -> void
	{
		glProgramUniform1ui(m_Program, m_UniformLocations.find(name)->second, value);
	}

	auto ShaderProgram2::Uniform(std::string_view const name, f32 const value) const -> void
	{
		glProgramUniform1f(m_Program, m_UniformLocations.find(name)->second, value);
	}

	auto ShaderProgram2::Uniform(std::string_view const name, Vector3 const& value) const -> void
	{
		glProgramUniform3fv(m_Program, m_UniformLocations.find(name)->second, 1, value.Data().data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, Matrix4 const& value) const -> void
	{
		glProgramUniformMatrix4fv(m_Program, m_UniformLocations.find(name)->second, 1, GL_TRUE, value.Data()[0].Data().data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, std::span<i32 const> const values) const -> void
	{
		glProgramUniform1iv(m_Program, m_UniformLocations.find(name)->second, values.size(), values.data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, std::span<u32 const> const values) const -> void
	{
		glProgramUniform1uiv(m_Program, m_UniformLocations.find(name)->second, values.size(), values.data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, std::span<f32 const> const values) const -> void
	{
		glProgramUniform1fv(m_Program, m_UniformLocations.find(name)->second, values.size(), values.data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, std::span<Vector3 const> const values) const -> void
	{
		glProgramUniform3fv(m_Program, m_UniformLocations.find(name)->second, values.size(), values.data()->Data().data());
	}

	auto ShaderProgram2::Uniform(std::string_view const name, std::span<Matrix4 const> const values) const -> void
	{
		glProgramUniformMatrix4fv(m_Program, m_UniformLocations.find(name)->second, values.size(), GL_TRUE, values.data()->Data().data()->Data().data());
	}

	auto ShaderProgram2::Binary() const -> ShaderProgramCachedBinaryOutputInfo
	{
		GLint binaryLength;
		glGetProgramiv(m_Program, GL_PROGRAM_BINARY_LENGTH, &binaryLength);

		ShaderProgramCachedBinaryOutputInfo info;
		info.binary.resize(binaryLength);

		glGetProgramBinary(m_Program, binaryLength, nullptr, &info.format, info.binary.data());
		return info;
	}

	auto ShaderProgram2::Use() const -> void
	{
		glUseProgram(m_Program);
	}

	auto ShaderProgram2::InternalHandle() const -> u32
	{
		return m_Program;
	}

	auto ShaderProgram2::QueryUniformLocations() -> void
	{
		GLint numUniforms;
		glGetProgramInterfaceiv(m_Program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

		GLint maxUniformNameLength;
		glGetProgramInterfaceiv(m_Program, GL_UNIFORM, GL_MAX_NAME_LENGTH, &maxUniformNameLength);

		std::string uniformName(maxUniformNameLength, 0);

		for (auto i = 0; i < numUniforms; i++)
		{
			GLenum constexpr props[]{GL_LOCATION};
			GLint uniformLocation;
			glGetProgramResourceiv(m_Program, GL_UNIFORM, i, 1, props, 0, nullptr, &uniformLocation);
			GLsizei uniformNameLength;
			glGetProgramResourceName(m_Program, GL_UNIFORM, i, maxUniformNameLength, &uniformNameLength, uniformName.data());
			m_UniformLocations[uniformName.substr(0, uniformNameLength)] = uniformLocation;
		}
	}

	ShaderProgram2::~ShaderProgram2() noexcept
	{
		glDeleteShader(m_VertexShader);
		glDeleteShader(m_GeometryShader);
		glDeleteShader(m_FragmentShader);
		glDeleteProgram(m_Program);
	}
}
