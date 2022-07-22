#include "rendering/ShaderProgram.hpp"

#include "Logger.hpp"
#include "Util.hpp"
#include "rendering/gl/GlCore.hpp"

#include <format>


namespace leopph
{
	ShaderProgram::ShaderProgram(ShaderProgramSourceInfo const& sourceInfo) :
		mProgram{glCreateProgram()}
	{
		auto const vertexShader = glCreateShader(GL_VERTEX_SHADER);

		if (auto const infoLog = compile_shader(vertexShader, sourceInfo.vertex))
		{
			internal::Logger::Instance().Error(std::format("Error while compiling shader: vertex shader failed to compile, reason: {}.", *infoLog));
			glDeleteShader(vertexShader);
			return;
		}

		glAttachShader(mProgram, vertexShader);
		glDeleteShader(vertexShader);

		if (sourceInfo.geometry)
		{
			auto const geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

			if (auto const infoLog = compile_shader(geometryShader, *sourceInfo.geometry))
			{
				internal::Logger::Instance().Error(std::format("Error while compiling shader: geometry shader failed to compile, reason: {}.", *infoLog));
				glDeleteShader(geometryShader);
				return;
			}

			glAttachShader(mProgram, geometryShader);
			glDeleteShader(geometryShader);
		}

		if (sourceInfo.fragment)
		{
			auto const fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

			if (auto const infoLog = compile_shader(fragmentShader, *sourceInfo.fragment))
			{
				internal::Logger::Instance().Error(std::format("Error while compiling shader: fragment shader failed to compile, reason: {}.", *infoLog));
				glDeleteShader(fragmentShader);
				return;
			}

			glAttachShader(mProgram, fragmentShader);
			glDeleteShader(fragmentShader);
		}

		if (auto const infoLog = link_program(mProgram))
		{
			internal::Logger::Instance().Error(std::format("Error while compiling shader: program failed to link, reason: {}.", *infoLog));
			glDeleteProgram(mProgram);
			mProgram = 0;
			return;
		}

		GLint numUniforms;
		glGetProgramInterfaceiv(mProgram, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

		GLint maxUniformNameLength;
		glGetProgramInterfaceiv(mProgram, GL_UNIFORM, GL_MAX_NAME_LENGTH, &maxUniformNameLength);

		std::string uniformName(maxUniformNameLength, 0);

		for (auto i = 0; i < numUniforms; i++)
		{
			GLint uniformLocation;
			glGetProgramResourceiv(mProgram, GL_UNIFORM, i, 1, std::array<GLenum, 1>{GL_LOCATION}.data(), 1, nullptr, &uniformLocation);

			if (uniformLocation == -1)
			{
				continue;
			}

			GLsizei uniformNameLength;
			glGetProgramResourceName(mProgram, GL_UNIFORM, i, maxUniformNameLength, &uniformNameLength, uniformName.data());

			mUniformLocations[uniformName.substr(0, uniformNameLength)] = uniformLocation;
		}
	}



	void ShaderProgram::use() const
	{
		glUseProgram(mProgram);
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, bool const value)
	{
		glProgramUniform1i(program, location, static_cast<GLint>(value));
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, i32 const value)
	{
		glProgramUniform1i(program, location, value);
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, u32 const value)
	{
		glProgramUniform1ui(program, location, value);
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, f32 const value)
	{
		glProgramUniform1f(program, location, value);
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, Vector3 const& value)
	{
		glProgramUniform3fv(program, location, 1, value.Data().data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, Matrix4 const& value)
	{
		glProgramUniformMatrix4fv(program, location, 1, GL_TRUE, value.Data().data()->Data().data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<i32 const> const values)
	{
		glProgramUniform1iv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<u32 const> const values)
	{
		glProgramUniform1uiv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<f32 const> const values)
	{
		glProgramUniform1fv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<Vector3 const> const values)
	{
		glProgramUniform3fv(program, location, clamp_cast<GLsizei>(values.size()), values.data()->Data().data());
	}



	void ShaderProgram::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<Matrix4 const> const values)
	{
		glProgramUniformMatrix4fv(program, location, clamp_cast<GLsizei>(values.size()), GL_TRUE, values.data()->Data().data()->Data().data());
	}



	void ShaderProgram::log_invalid_uniform_access(std::string_view uniformName)
	{
		internal::Logger::Instance().Debug(std::format("Ignoring attempt to set shader uniform [{}]: the uniform does not exist in the shader program.", uniformName));
	}



	std::optional<std::string> ShaderProgram::compile_shader(u32 const shader, std::span<std::string const> const lines)
	{
		std::vector<char const*> linePtrs;
		linePtrs.reserve(lines.size());

		for (auto const& line : lines)
		{
			linePtrs.push_back(line.data());
		}

		glShaderSource(shader, clamp_cast<GLsizei>(linePtrs.size()), linePtrs.data(), nullptr);
		glCompileShader(shader);

		GLint result;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

		if (result == GL_TRUE)
		{
			return std::nullopt;
		}

		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);
		std::string infoLog(result, ' ');

		glGetShaderInfoLog(shader, result, nullptr, infoLog.data());
		return std::move(infoLog);
	}



	std::optional<std::string> ShaderProgram::link_program(u32 const program)
	{
		glLinkProgram(program);

		GLint result;
		glGetProgramiv(program, GL_LINK_STATUS, &result);

		if (result == GL_TRUE)
		{
			return std::nullopt;
		}

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &result);
		std::string infoLog(result, ' ');

		glGetProgramInfoLog(program, result, nullptr, infoLog.data());
		return std::move(infoLog);
	}



	ShaderProgram::~ShaderProgram()
	{
		glDeleteProgram(mProgram);
	}
}
