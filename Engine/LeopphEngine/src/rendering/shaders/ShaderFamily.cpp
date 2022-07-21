#include "rendering/shaders/ShaderFamily.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "Util.hpp"
#include "rendering/gl/GlCore.hpp"
#include "rendering/shaders/ShaderProcessing.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <stdexcept>
#include <Util.hpp>


namespace leopph
{
	ShaderFamily::ShaderFamily(ShaderProgramSourceInfo sourceInfo) :
		m_CurrentPermutationBits{0},
		m_SourceInfo{std::move(sourceInfo)}
	{
		auto const AddNewLineChars = [](std::vector<std::string>& lines)
		{
			for (auto& line : lines)
			{
				line.push_back('\n');
			}
		};

		AddNewLineChars(m_SourceInfo.vertex);

		if (m_SourceInfo.geometry)
		{
			AddNewLineChars(*m_SourceInfo.geometry);
		}

		if (m_SourceInfo.fragment)
		{
			AddNewLineChars(*m_SourceInfo.fragment);
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
		/*for (auto const& [name, option] : m_OptionsByName)
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
						newInfo.bitset[option.shift + j] = static_cast<bool>(value & (0x00000001 << j));
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

			QueryUniformLocations(perm);

			// Store permutation
			m_PermutationByBits.emplace(std::move(permInstance.bitset), std::move(perm));
		}*/
	}



	auto ShaderFamily::SelectPermutation(PermutationBitset const bitset) -> void
	{
		if (!m_PermutationByBits.contains(bitset))
		{
			if (!CompilePermutation(bitset))
			{
				internal::Logger::Instance().Error("Error while selecting permutation: the permutation was not ready, but an error occured while compiling it.");
				return;
			}
		}

		m_CurrentPermutationBits = bitset;
	}



	auto ShaderFamily::SetOption(std::string_view const name, u8 value) -> void
	{
		auto const it = m_OptionIndexByName.find(name);

		if (it == std::end(m_OptionIndexByName))
		{
			internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader option [{}]: the option was not specified in the shader.", name));
			return;
		}

		auto const& option = m_InstanceOptions[it->second];

		#ifndef NDEBUG
		if (option.min > value || option.max < value)
		{
			auto const errMsg = std::format("Error setting shader option value: value [{}] was out of range for option [{}].", value, name);
			internal::Logger::Instance().Error(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif

		auto const normValue = value - option.min;
		m_CurrentPermutationBits = (m_CurrentPermutationBits & (~option.mask)) | (normValue << option.shift);
	}



	auto ShaderFamily::AddGlobalOption(std::string_view const name, u8 const min, u8 const max) -> bool
	{
		auto const maxValueNorm = max - min + 1;
		auto const numBits = math::BinaryDigitCount(maxValueNorm);

		if (auto const nextShift = NextFreeGlobalShift(numBits))
		{
			PermutationBitset const mask = ((1 << numBits) - 1) << *nextShift;
			s_GlobalOptions.emplace_back(std::string{name}, mask, *nextShift, min, max);

			return true;
		}

		return false;
	}



	auto ShaderFamily::NextFreeGlobalShift(u8 const requiredBits) -> std::optional<u8>
	{
		auto nextShift = std::numeric_limits<PermutationBitset>::digits;

		for (auto const& option : s_GlobalOptions)
		{
			nextShift = std::min<decltype(nextShift)>(nextShift, option.shift);
		}

		if (nextShift - requiredBits >= 0)
		{
			return ClampCast<u8>(nextShift);
		}

		return std::nullopt;
	}



	auto ShaderFamily::NextFreeInstanceShift(u8 const requiredBits) const -> std::optional<u8>
	{
		auto nextShift{0};

		for (auto const& option : m_InstanceOptions)
		{
			nextShift = std::max(option.shift + - math::BinaryDigitCount(option.max - option.min + 1), nextShift);
		}

		if (nextShift + requiredBits < std::numeric_limits<PermutationBitset>::digits)
		{
			return ClampCast<u8>(nextShift);
		}

		return std::nullopt;
	}



	auto ShaderFamily::SetUniform(std::string_view const name, bool const value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, i32 const value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, u32 const value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, f32 const value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, Vector3 const& value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, Matrix4 const& value) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
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



	auto ShaderFamily::SetUniform(std::string_view const name, std::span<i32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1iv(permutation.program, uniformIt->second, ClampCast<GLsizei>(values.size()), values.data());
	}



	auto ShaderFamily::SetUniform(std::string_view const name, std::span<u32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1uiv(permutation.program, uniformIt->second, ClampCast<GLsizei>(values.size()), values.data());
	}



	auto ShaderFamily::SetUniform(std::string_view const name, std::span<f32 const> const values) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform1fv(permutation.program, uniformIt->second, ClampCast<GLsizei>(values.size()), values.data());
	}



	auto ShaderFamily::SetUniform(std::string_view const name, std::span<Vector3 const> const values) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniform3fv(permutation.program, uniformIt->second, ClampCast<GLsizei>(values.size()), values.data()->Data().data());
	}



	auto ShaderFamily::SetUniform(std::string_view const name, std::span<Matrix4 const> const values) const -> void
	{
		auto const& permutation = m_PermutationByBits.find(m_CurrentPermutationBits)->second;
		auto const uniformIt = permutation.uniformLocations.find(name);

		#ifndef NDEBUG
		if (uniformIt == std::end(permutation.uniformLocations))
		{
			LogInvalidUniformAccess(name);
			return;
		}
		#endif

		glProgramUniformMatrix4fv(permutation.program, uniformIt->second, ClampCast<GLsizei>(values.size()), GL_TRUE, values.data()->Data().data()->Data().data());
	}



	auto ShaderFamily::UseCurrentPermutation() const -> void
	{
		glUseProgram(m_PermutationByBits.find(m_CurrentPermutationBits)->second.program);
	}



	auto ShaderFamily::LogInvalidUniformAccess(std::string_view const name) -> void
	{
		internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader uniform [{}]: the uniform does not exist in the current permutation.", name));
	}



	auto ShaderFamily::CompilePermutation(PermutationBitset const bitset) -> bool
	{
		u32 usedLocalBits{0};

		for (auto const& instanceOption : m_InstanceOptions)
		{
			usedLocalBits = ClampCast<u32>(std::max<i64>(usedLocalBits, math::BinaryDigitCount(instanceOption.max - instanceOption.min + 1) + instanceOption.shift));
		}

		u32 usedGlobalBits{0};

		for (auto const& globalOption : s_GlobalOptions)
		{
			usedGlobalBits = std::min<u32>(usedGlobalBits, globalOption.shift);
		}

		if (usedLocalBits + usedGlobalBits > std::numeric_limits<PermutationBitset>::digits)
		{
			internal::Logger::Instance().Error("Error while compiling shader permutation: too many options in total.");
			return false;
		}

		std::vector<std::string> optionDefines;

		for (auto const& options : {std::cref(m_InstanceOptions), std::cref(s_GlobalOptions)})
		{
			for (auto const& option : options.get())
			{
				auto const optionValue = (option.max & bitset) + option.min;

				if (optionValue > option.max)
				{
					internal::Logger::Instance().Error(std::format("Error while compiling shader permutation: value [{}] for option [{}] is higher than its maximum [{}].", optionValue, option.name, option.max));
					return false;
				}

				optionDefines.push_back(std::format("#define {} {}\n", option.name, optionValue));
			}
		}

		auto sourceInfo = m_SourceInfo;
		std::string versionLine = "#version 450 core\n";

		auto const program = glCreateProgram();

		if (sourceInfo.fragment)
		{
			sourceInfo.fragment->insert(std::begin(*sourceInfo.fragment), std::begin(optionDefines), std::end(optionDefines));
			sourceInfo.fragment->insert(std::begin(*sourceInfo.fragment), versionLine);

			auto const fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

			if (auto const infoLog = CompileShader(fragmentShader, *sourceInfo.fragment))
			{
				internal::Logger::Instance().Error(std::format("Error while compiling permutation: fragment shader failed to compile, reason: {}.", *infoLog));
				glDeleteShader(fragmentShader);
				glDeleteProgram(program);
				return false;
			}

			glAttachShader(program, fragmentShader);
			glDeleteShader(fragmentShader);
		}

		if (sourceInfo.geometry)
		{
			sourceInfo.geometry->insert(std::begin(*sourceInfo.geometry), std::begin(optionDefines), std::end(optionDefines));
			sourceInfo.geometry->insert(std::begin(*sourceInfo.geometry), versionLine);

			auto const geometryShader = glCreateShader(GL_GEOMETRY_SHADER);

			if (auto const infoLog = CompileShader(geometryShader, *sourceInfo.geometry))
			{
				internal::Logger::Instance().Error(std::format("Error while compiling permutation: geometry shader failed to compile, reason: {}.", *infoLog));
				glDeleteShader(geometryShader);
				glDeleteProgram(program);
				return false;
			}

			glAttachShader(program, geometryShader);
			glDeleteShader(geometryShader);
		}

		sourceInfo.vertex.insert(std::begin(sourceInfo.vertex), std::make_move_iterator(std::begin(optionDefines)), std::make_move_iterator(std::end(optionDefines)));
		sourceInfo.vertex.insert(std::begin(sourceInfo.vertex), std::move(versionLine));

		auto const vertexShader = glCreateShader(GL_VERTEX_SHADER);

		if (auto const infoLog = CompileShader(vertexShader, sourceInfo.vertex))
		{
			internal::Logger::Instance().Error(std::format("Error while compiling permutation: vertex shader failed to compile, reason: {}.", *infoLog));
			glDeleteShader(vertexShader);
			glDeleteProgram(program);
			return false;
		}

		glAttachShader(program, vertexShader);
		glDeleteShader(vertexShader);

		if (auto const infoLog = LinkProgram(program))
		{
			internal::Logger::Instance().Error(std::format("Error while compiling permutation: program failed to link, reason: {}.", *infoLog));
			glDeleteProgram(program);
			return false;
		}

		auto& perm = m_PermutationByBits.emplace(bitset, program).first->second;
		QueryUniformLocations(perm);

		return true;
	}



	auto ShaderFamily::CompileShader(u32 const shader, std::span<std::string const> const lines) -> std::optional<std::string>
	{
		std::vector<char const*> linePtrs;
		linePtrs.reserve(lines.size());

		for (auto const& line : lines)
		{
			linePtrs.push_back(line.data());
		}

		glShaderSource(shader, ClampCast<GLsizei>(linePtrs.size()), linePtrs.data(), nullptr);
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



	auto ShaderFamily::LinkProgram(u32 const program) -> std::optional<std::string>
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



	auto ShaderFamily::QueryUniformLocations(Permutation& perm) -> void
	{
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
	}



	ShaderFamily::~ShaderFamily()
	{
		for (auto const& permutation : m_PermutationByBits | std::views::values)
		{
			glDeleteProgram(permutation.program);
		}
	}



	auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath) -> ShaderFamily
	{
		auto fileInfo = ReadShaderFiles(std::move(vertexShaderPath), std::move(geometryShaderPath), std::move(fragmentShaderPath));
		auto sourceInfo = ProcessShaderIncludes(std::move(fileInfo));
		return ShaderFamily{std::move(sourceInfo)};
	}
}
