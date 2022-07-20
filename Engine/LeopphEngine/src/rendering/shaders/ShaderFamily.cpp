#include "rendering/shaders/ShaderFamily.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "Util.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/shaders/ShaderProcessing.hpp"

#include <format>
#include <iterator>
#include <optional>
#include <ranges>
#include <regex>
#include <stdexcept>


namespace leopph
{
	ShaderFamily::ShaderFamily(ShaderProgramSourceInfo sourceInfo, std::vector<ShaderOptionInfo> options)
	{
		auto const AddNewLineChars = [](std::vector<std::string>& lines)
		{
			for (auto& line : lines)
			{
				line.push_back('\n');
			}
		};

		AddNewLineChars(sourceInfo.vertex);

		if (sourceInfo.geometry)
		{
			AddNewLineChars(*sourceInfo.geometry);
		}

		if (sourceInfo.fragment)
		{
			AddNewLineChars(*sourceInfo.fragment);
		}

		// A concrete state of an option
		struct OptionInstance
		{
			std::string_view name;
			u32 value;
		};

		// A permutation represented by a bitset and the option instances associated with it
		struct PermutationInstance
		{
			std::vector<bool> bitset;
			std::vector<OptionInstance> optInstances;
		};

		// Accumulator buffer for all permutations
		std::vector<PermutationInstance> permutationInstances;

		// Generate option instances for the default (all 0 bits) permutation

		std::vector<OptionInstance> defaultOptInstances;
		for (auto const& [name, option] : m_OptionsByName)
		{
			defaultOptInstances.emplace_back(name, option.min);
		}

		// Insert the default instance into the accumulator buffer
		permutationInstances.emplace_back(m_CurrentPermutationBits, defaultOptInstances);

		// Permutate the already created instances

		std::vector<PermutationInstance> tmpBuffer;

		for (auto const& [name, option] : m_OptionsByName)
		{
			// Find the option instance id

			auto permInstanceIndex = 0;

			for (auto i = 0; i < permutationInstances[0].optInstances.size(); i++)
			{
				if (permutationInstances[0].optInstances[i].name == name)
				{
					permInstanceIndex = i;
					break;
				}
			}

			tmpBuffer.clear();

			for (auto const& permInfo : permutationInstances)
			{
				auto const range = option.max - option.min;

				// Go through all possible values of the option, generate the bitset and set the value corresponding to the bitset

				for (u32 value = 1; value <= range; value++)
				{
					auto newInfo = permInfo;
					newInfo.optInstances[permInstanceIndex].value = option.min + value;

					auto const digits = math::BinaryDigitCount(value);
					for (u8 j = 0; j < digits; j++)
					{
						newInfo.bitset[option.index + j] = static_cast<bool>(value & (0x00000001 << j));
					}

					tmpBuffer.emplace_back(std::move(newInfo));
				}
			}

			permutationInstances.insert(std::end(permutationInstances), std::make_move_iterator(std::begin(tmpBuffer)), std::make_move_iterator(std::end(tmpBuffer)));
		}

		// Build the shaders permutations

		std::vector<std::string> defines;
		std::vector<char const*> linePtrs;

		for (auto& permInstance : permutationInstances)
		{
			auto const* const versionSpecifier = "#version 460 core\n";

			defines.clear();

			// Generate the define string for each option
			for (auto const& [name, value] : permInstance.optInstances)
			{
				defines.push_back(std::string{"#define "}.append(name).append(1, ' ').append(std::to_string(value)).append(1, '\n'));
			}

			linePtrs.clear();

			// Insert the glsl version specifier into the first line
			linePtrs.push_back(versionSpecifier);

			// Collect the C pointers to the define strings

			for (auto const& defineLine : defines)
			{
				linePtrs.push_back(defineLine.data());
			}

			// Save the number of common lines for later
			auto const numCommonLines = linePtrs.size();

			Permutation perm;

			perm.program = glCreateProgram();

			// Add vertex shader lines
			for (auto const& line : m_VertexSource)
			{
				linePtrs.push_back(line.data());
			}

			auto const CompileShader = [](GLuint const shader, std::vector<char const*> const& lines) -> void
			{
				glShaderSource(shader, lines.size(), lines.data(), nullptr);
				glCompileShader(shader);
			};

			auto const CheckCompileError = [](GLuint const shader) -> std::optional<std::string>
			{
				GLint result;
				glGetShaderiv(shader, GL_COMPILE_STATUS, &result);

				if (result == GL_TRUE)
				{
					return std::nullopt;
				}

				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);
				std::string infoLog(result, ' ');
				glGetShaderInfoLog(shader, result, nullptr, infoLog.data());
				return infoLog;
			};

			perm.vertex = glCreateShader(GL_VERTEX_SHADER);
			CompileShader(perm.vertex, linePtrs);

			#ifndef NDEBUG
			if (auto const info = CheckCompileError(perm.vertex))
			{
				internal::Logger::Instance().Error(std::format("Error compiling vertex shader: {}.", *info));
			}
			#endif

			glAttachShader(perm.program, perm.vertex);

			if (m_GeometrySource)
			{
				// Erase vertex shader lines, keep common lines
				linePtrs.resize(numCommonLines);

				// Add geometry shader lines
				for (auto const& line : *m_GeometrySource)
				{
					linePtrs.push_back(line.data());
				}

				perm.geometry = glCreateShader(GL_GEOMETRY_SHADER);
				CompileShader(perm.geometry, linePtrs);

				#ifndef NDEBUG
				if (auto const info = CheckCompileError(perm.geometry))
				{
					internal::Logger::Instance().Error(std::format("Error compiling geometry shader: {}.", *info));
				}
				#endif

				glAttachShader(perm.program, perm.geometry);
			}

			if (m_FragmentSource)
			{
				// Erase vertex shader lines, keep common lines
				linePtrs.resize(numCommonLines);

				// Add fragment shader lines
				for (auto const& line : *m_FragmentSource)
				{
					linePtrs.push_back(line.data());
				}

				perm.fragment = glCreateShader(GL_FRAGMENT_SHADER);
				CompileShader(perm.fragment, linePtrs);

				#ifndef NDEBUG
				if (auto const info = CheckCompileError(perm.fragment))
				{
					internal::Logger::Instance().Error(std::format("Error compiling fragment shader: {}.", *info));
				}
				#endif

				glAttachShader(perm.program, perm.fragment);
			}

			glLinkProgram(perm.program);

			GLint glResult;
			glGetProgramiv(perm.program, GL_LINK_STATUS, &glResult);

			if (!glResult)
			{
				glGetProgramiv(perm.program, GL_INFO_LOG_LENGTH, &glResult);
				std::string infoLog(glResult, ' ');
				glGetProgramInfoLog(perm.program, glResult, nullptr, infoLog.data());
				internal::Logger::Instance().Error(std::format("Error linking shader program: {}.", infoLog));
			}

			// Query uniform locations

			GLint numUniforms;
			glGetProgramInterfaceiv(perm.program, GL_UNIFORM, GL_ACTIVE_RESOURCES, &numUniforms);

			GLint maxUniformNameLength;
			glGetProgramInterfaceiv(perm.program, GL_UNIFORM, GL_MAX_NAME_LENGTH, &maxUniformNameLength);

			std::string uniformName(maxUniformNameLength, 0);

			for (auto i = 0; i < numUniforms; i++)
			{
				GLenum constexpr props[]{GL_LOCATION};
				GLint uniformLocation;
				glGetProgramResourceiv(perm.program, GL_UNIFORM, i, 1, props, 0, nullptr, &uniformLocation);
				GLsizei uniformNameLength;
				glGetProgramResourceName(perm.program, GL_UNIFORM, i, maxUniformNameLength, &uniformNameLength, uniformName.data());
				perm.uniformLocations[uniformName.substr(0, uniformNameLength)] = uniformLocation;
			}

			// Store permutation
			m_PermutationsByBits.emplace(std::move(permInstance.bitset), std::move(perm));
		}
	}



	auto ShaderFamily::Option(std::string_view const name, u8 value) -> void
	{
		auto const it = m_OptionsByName.find(name);

		if (it == std::end(m_OptionsByName))
		{
			#ifndef NDEBUG
			internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader option [{}]: the option was not specified in the shader.", name));
			#endif
			return;
		}

		auto const& option = it->second;

		#ifndef NDEBUG
		if (option.min > value || option.max < value)
		{
			auto const errMsg = std::format("Error setting shader option value: value [{}] was out of range for option [{}].", value, name);
			internal::Logger::Instance().Error(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif

		auto const normValue = value - option.min;
		auto const range = option.max - option.min + 1;
		auto const numBits = math::BinaryDigitCount(range);

		u32 mask{0};

		for (u8 i = 0; i < numBits; i++)
		{
			mask |= 1 << (i + option.index);
		}

		m_CurrentPermutationBits = (m_CurrentPermutationBits & (~mask)) | (normValue << option.index);
	}



	auto ShaderFamily::Uniform(std::string_view const name, bool const value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1i(permutation.program, uniformIt->second, static_cast<GLint>(value));
	}



	auto ShaderFamily::Uniform(std::string_view const name, i32 const value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1i(permutation.program, uniformIt->second, value);
	}



	auto ShaderFamily::Uniform(std::string_view const name, u32 const value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1ui(permutation.program, uniformIt->second, value);
	}



	auto ShaderFamily::Uniform(std::string_view const name, f32 const value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1f(permutation.program, uniformIt->second, value);
	}



	auto ShaderFamily::Uniform(std::string_view const name, Vector3 const& value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform3fv(permutation.program, uniformIt->second, 1, value.Data().data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, Matrix4 const& value) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniformMatrix4fv(permutation.program, uniformIt->second, 1, GL_TRUE, value.Data()[0].Data().data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, std::span<i32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1iv(permutation.program, uniformIt->second, values.size(), values.data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, std::span<u32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1uiv(permutation.program, uniformIt->second, values.size(), values.data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, std::span<f32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1fv(permutation.program, uniformIt->second, values.size(), values.data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, std::span<Vector3 const> const values) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform3fv(permutation.program, uniformIt->second, values.size(), values.data()->Data().data());
	}



	auto ShaderFamily::Uniform(std::string_view const name, std::span<Matrix4 const> const values) const -> void
	{
		auto const& permutation = m_PermutationsByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniformMatrix4fv(permutation.program, uniformIt->second, values.size(), GL_TRUE, values.data()->Data().data()->Data().data());
	}



	auto ShaderFamily::UseCurrentPermutation() const -> void
	{
		glUseProgram(m_PermutationsByBits.find(m_CurrentPermutationBits)->second.program);
	}



	auto ShaderFamily::LogInvalidUniformAccess(std::string_view const name) -> void
	{
		internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader uniform [{}]: the uniform does not exist in the current permutation.", name));
	}



	auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath) -> ShaderFamily
	{
		auto fileInfo = ReadShaderFiles(std::move(vertexShaderPath), std::move(geometryShaderPath), std::move(fragmentShaderPath));
		auto sourceInfo = ProcessShaderIncludes(std::move(fileInfo));
		return ShaderFamily{std::move(sourceInfo)};
	}
}
