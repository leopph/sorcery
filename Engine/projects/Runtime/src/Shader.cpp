#include "Shader.hpp"

#include "GlContext.hpp"
#include "Logger.hpp"
#include "Util.hpp"

#include <algorithm>
#include <format>
#include <iterator>
#include <regex>


namespace leopph
{
	Shader::Shader(std::filesystem::path const& path) :
		Shader{add_new_line_chars(process_includes(read_source_file(path)))}
	{}



	Shader::Shader(ProcessedSourceLineView const sourceLines) :
		mProgram{glCreateProgram()}
	{
		// Collect pointers to the underlying strings and add the necessary lines

		std::vector<char const*> linePtrs;
		linePtrs.push_back(VERSION_LINE);
		linePtrs.push_back(BINDLESS_EXT_LINE);
		linePtrs.push_back(VERTEX_DEFINE_LINE);

		for (auto const& line : sourceLines)
		{
			linePtrs.emplace_back(line.data());
		}

		// Compile vertex shader

		auto const vertexShader = glCreateShader(GL_VERTEX_SHADER);

		if (auto const infoLog = compile_shader(vertexShader, linePtrs))
		{
			Logger::get_instance().error(std::format("Error while compiling shader: vertex shader failed to compile, reason: {}", *infoLog));
			glDeleteShader(vertexShader);
			return;
		}

		glAttachShader(mProgram, vertexShader);
		glDeleteShader(vertexShader);

		// Compile fragment shader

		linePtrs[2] = FRAGMENT_DEFINE_LINE;
		auto const fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		if (auto const infoLog = compile_shader(fragmentShader, linePtrs))
		{
			Logger::get_instance().error(std::format("Error while compiling shader: fragment shader failed to compile, reason: {}", *infoLog));
			glDeleteShader(fragmentShader);
			return;
		}

		glAttachShader(mProgram, fragmentShader);
		glDeleteShader(fragmentShader);

		// Link program

		if (auto const infoLog = link_program(mProgram))
		{
			Logger::get_instance().error(std::format("Error while compiling shader: program failed to link, reason: {}", *infoLog));
			glDeleteProgram(mProgram);
			mProgram = 0;
			return;
		}

		// Query uniforms

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



	void Shader::use() const
	{
		glUseProgram(mProgram);
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, bool const value)
	{
		glProgramUniform1i(program, location, static_cast<GLint>(value));
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, i32 const value)
	{
		glProgramUniform1i(program, location, value);
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, u32 const value)
	{
		glProgramUniform1ui(program, location, value);
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, f32 const value)
	{
		glProgramUniform1f(program, location, value);
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, Vector3 const& value)
	{
		glProgramUniform3fv(program, location, 1, value.get_data().data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, Matrix4 const& value)
	{
		glProgramUniformMatrix4fv(program, location, 1, GL_TRUE, value.get_data().data()->get_data().data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<i32 const> const values)
	{
		glProgramUniform1iv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<u32 const> const values)
	{
		glProgramUniform1uiv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<f32 const> const values)
	{
		glProgramUniform1fv(program, location, clamp_cast<GLsizei>(values.size()), values.data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<Vector3 const> const values)
	{
		glProgramUniform3fv(program, location, clamp_cast<GLsizei>(values.size()), values.data()->get_data().data());
	}



	void Shader::set_uniform_internal_concrete(u32 const program, i32 const location, std::span<Matrix4 const> const values)
	{
		glProgramUniformMatrix4fv(program, location, clamp_cast<GLsizei>(values.size()), GL_TRUE, values.data()->get_data().data()->get_data().data());
	}



	void Shader::log_invalid_uniform_access(std::string_view uniformName)
	{
		Logger::get_instance().debug(std::format("Ignoring attempt to set shader uniform [{}]: the uniform does not exist in the shader program.", uniformName));
	}



	Shader::SourceFileInfo Shader::read_source_file(std::filesystem::path const& path)
	{
		if (path.empty())
		{
			Logger::get_instance().error("Error reading shader file: shader path was empty.");
			return {};
		}

		if (!exists(path))
		{
			Logger::get_instance().error("Error reading shader file: the file does not exist.");
			return {};
		}

		SourceFileInfo fileInfo;
		fileInfo.absolutePath = absolute(path).make_preferred();
		read_file_lines(fileInfo.absolutePath, fileInfo.lines);
		return fileInfo;
	}



	void Shader::process_includes_recursive(std::filesystem::path const& absolutePath, Shader::RawSourceLines& lines)
	{
		std::regex const static includeLineRegex{R"delim(^\s*#\s*include\s*)delim"};
		std::regex const static includeLineQuoteRegex{R"delim(^\s*#\s*include\s*"\S+"\s*$)delim"};
		std::regex const static includeLineBracketRegex{R"delim(^\s*#\s*include\s*<\S+>\s*$)delim"};
		std::regex const static fileNameInQuotesRegex{R"delim("\S+")delim"};
		std::regex const static fileNameInBracketsRegex{R"delim(<\S+>)delim"};

		std::vector<std::string> inclFileBuf;

		// Index based loop because we will potentially insert lines into the vector
		for (std::size_t i{0}; i < lines.size(); i++)
		{
			// starts with #include
			if (std::regex_search(lines[i], includeLineRegex))
			{
				std::string includeFileName;

				auto const getFileNameFromLine = [i, &lines](std::regex const& nameRegex) -> std::string
				{
					std::regex_iterator const regIt{std::begin(lines[i]), std::end(lines[i]), nameRegex};
					return regIt->str().substr(1, regIt->length() - 2);
				};

				// #include "file"
				if (std::regex_match(lines[i], includeLineQuoteRegex))
				{
					includeFileName = getFileNameFromLine(fileNameInQuotesRegex);
				}
				// #include <file>
				else if (std::regex_match(lines[i], includeLineBracketRegex))
				{
					includeFileName = getFileNameFromLine(fileNameInBracketsRegex);
				}
				else
				{
					// We couldn't parse the include directive so we log the error, clear the erroneous lines[i], and move on.
					Logger::get_instance().error("Error parsing shader includes: ill-formed file specifier was found.");
					lines[i].clear();
					i++;
					continue;
				}

				auto const includePath = absolute(absolutePath.parent_path() / includeFileName).make_preferred();

				inclFileBuf.clear();
				read_file_lines(includePath, inclFileBuf);

				if (inclFileBuf.empty())
				{
					// The the file to be included was either empty, or does not exist.
					Logger::get_instance().error(std::format("Error parsing shader includes: included file [{}] was empty or does not exist.", includePath.string()));
				}

				process_includes_recursive(includePath, inclFileBuf);

				// Clear include lines[i]
				lines[i].clear();

				// Add included file lines at the include directive's position
				lines.insert(std::begin(lines) + i, std::make_move_iterator(std::begin(inclFileBuf)), std::make_move_iterator(std::end(inclFileBuf)));

				// We don't increment here because the current index points to the first lines[i] of the included file, which we are yet to examine
			}
		}
	}



	Shader::ProcessedSourceLines Shader::process_includes(SourceFileInfo fileInfo)
	{
		process_includes_recursive(fileInfo.absolutePath, fileInfo.lines);
		return std::move(fileInfo.lines);
	}



	Shader::ProcessedSourceLines Shader::add_new_line_chars(ProcessedSourceLines sourceLines)
	{
		for (auto& line : sourceLines)
		{
			line.push_back('\n');
		}

		return std::move(sourceLines);
	}



	std::optional<std::string> Shader::compile_shader(u32 const shader, std::span<char const* const> const lines)
	{
		glShaderSource(shader, clamp_cast<GLsizei>(lines.size()), lines.data(), nullptr);
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



	std::optional<std::string> Shader::link_program(u32 const program)
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



	Shader::~Shader()
	{
		glDeleteProgram(mProgram);
	}



	char const* const Shader::VERSION_LINE{"#version 460 core\n"};
	char const* const Shader::BINDLESS_EXT_LINE{"#extension GL_ARB_bindless_texture : require\n"};
	char const* const Shader::VERTEX_DEFINE_LINE{"#define VERTEX_SHADER 1\n"};
	char const* const Shader::FRAGMENT_DEFINE_LINE{"#define FRAGMENT_SHADER 1\n"};
}
