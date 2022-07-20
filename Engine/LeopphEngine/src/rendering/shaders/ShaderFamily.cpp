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
	ShaderFamily::ShaderFamily(ShaderProgramSourceInfo const& sourceInfo) :
		m_VertexSource{sourceInfo.vertex},
		m_GeometrySource{sourceInfo.geometry},
		m_FragmentSource{sourceInfo.fragment}
	{
		auto const AddNewLineChars = [](std::vector<std::string>& lines)
		{
			for (auto& line : lines)
			{
				line.push_back('\n');
			}
		};

		// Extract options from sources and accumulate the required number of bits

		auto bitsRequired = ExtractOptions(m_VertexSource, m_Options);
		AddNewLineChars(m_VertexSource);

		if (m_GeometrySource)
		{
			bitsRequired += ExtractOptions(*m_GeometrySource, m_Options);
			AddNewLineChars(*m_GeometrySource);
		}

		if (m_FragmentSource)
		{
			bitsRequired += ExtractOptions(*m_FragmentSource, m_Options);
			AddNewLineChars(*m_FragmentSource);
		}

		// Default initalize a default permutation bitset with everything set to false
		m_OptionBits.resize(bitsRequired, false);

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
		for (auto const& [name, option] : m_Options)
		{
			defaultOptInstances.emplace_back(name, option.min);
		}

		// Insert the default instance into the accumulator buffer
		permutationInstances.emplace_back(m_OptionBits, defaultOptInstances);

		// Permutate the already created instances

		std::vector<PermutationInstance> tmpBuffer;

		for (auto const& [name, option] : m_Options)
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
						newInfo.bitset[option.id + j] = static_cast<bool>(value & (0x00000001 << j));
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

			auto const CompileShader = [](GLuint const shader, std::vector<char const*> const& lines) -> std::optional<std::string>
			{
				glShaderSource(shader, lines.size(), lines.data(), nullptr);
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
				return infoLog;
			};

			perm.vertex = glCreateShader(GL_VERTEX_SHADER);

			if (auto const info = CompileShader(perm.vertex, linePtrs))
			{
				internal::Logger::Instance().Error(std::format("Error compiling vertex shader: {}.", *info));
			}

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

				if (auto const info = CompileShader(perm.geometry, linePtrs))
				{
					internal::Logger::Instance().Error(std::format("Error compiling geometry shader: {}.", *info));
				}

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

				if (auto const info = CompileShader(perm.fragment, linePtrs))
				{
					internal::Logger::Instance().Error(std::format("Error compiling fragment shader: {}.", *info));
				}

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
			m_Permutations.emplace(std::move(permInstance.bitset), std::move(perm));
		}
	}



	auto ShaderFamily::Option(std::string_view const name, u32 const value) -> void
	{
		auto const it = m_Options.find(name);

		if (it == std::end(m_Options))
		{
			#ifndef NDEBUG
			internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader option [{}]: the option was not specified in the shader.", name));
			#endif
			return;
		}

		auto const& [min, max, id] = it->second;

		#ifndef NDEBUG
		if (min > value || max < value)
		{
			auto const errMsg = "Value [" + std::to_string(value) + "] was out of range for shader option [" + std::string{name} + "].";
			internal::Logger::Instance().Error(errMsg);
			throw std::runtime_error{errMsg};
		}
		#endif

		auto const normValue = value - min;
		auto const digits = math::BinaryDigitCount(normValue);

		for (auto i = 0; i < digits; i++)
		{
			m_OptionBits[id + i] = static_cast<bool>(normValue & (0x00000001 << i));
		}
	}



	auto ShaderFamily::Uniform(std::string_view const name, bool const value) const -> void
	{
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		auto const& permutation = m_Permutations.find(m_OptionBits)->second;
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
		glUseProgram(m_Permutations.find(m_OptionBits)->second.program);
	}



	auto ShaderFamily::ExtractOptions(std::vector<std::string>& sourceLines, std::unordered_map<std::string, ShaderOption, StringHash, StringEqual>& out) -> u32
	{
		std::regex const static validFullOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s+min\s*=\s*\d+\s+max\s*=\s*\d+\s*)delim"};
		std::regex const static validShortOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s*)delim"};
		std::regex const static nameAssignment{R"delim(=\s*[A-Za-z][\S]*)delim"};
		std::regex const static numberAssignment{R"delim(=\s*\d+)delim"};
		std::regex const static nameIdentifier{R"delim([A-Za-z][\S]*)delim"};
		std::regex const static number{R"delim(\d+)delim"};

		u32 nextFreeId{0};

		for (auto& line : sourceLines)
		{
			if (std::regex_search(line, validFullOptionRegex))
			{
				std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
				auto const assignmentStr = nameAssignmentMatch->str();
				std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};
				auto const name = nameMatch->str();

				std::regex_iterator numberAssignmentMatch{std::begin(line), std::end(line), numberAssignment};
				auto const minAssignmentStr = numberAssignmentMatch->str();
				std::regex_iterator minMatch{std::begin(minAssignmentStr), std::end(minAssignmentStr), number};
				u32 const min = std::stoi(minMatch->str());

				++numberAssignmentMatch;
				auto const maxAssignmentStr = numberAssignmentMatch->str();
				std::regex_iterator maxMatch{std::begin(maxAssignmentStr), std::end(maxAssignmentStr), number};
				u32 const max = std::stoi(maxMatch->str());

				ShaderOption option
				{
					.min = min,
					.max = max,
					.id = nextFreeId
				};

				if (out.contains(name))
				{
					if (out[name].min != option.min && out[name].max != option.max)
					{
						auto const errMsg = "Duplicate shader option [" + name + "] found while parsing options.";
						internal::Logger::Instance().Error(errMsg);
						throw std::runtime_error{errMsg};
					}
				}
				else
				{
					out[name] = option;
					nextFreeId += math::BinaryDigitCount(max - min);
				}

				line.clear();
			}
			else if (std::regex_search(line, validShortOptionRegex))
			{
				std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
				auto const assignmentStr = nameAssignmentMatch->str();
				std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};
				auto const name = nameMatch->str();

				ShaderOption option
				{
					.min = 0,
					.max = 1,
					.id = nextFreeId
				};

				if (out.contains(name))
				{
					if (out[name].min != option.min && out[name].max != option.max)
					{
						auto const errMsg = "Duplicate shader option [" + name + "] found while parsing options.";
						internal::Logger::Instance().Error(errMsg);
						throw std::runtime_error{errMsg};
					}
				}
				else
				{
					out[name] = option;
					nextFreeId += 1;
				}

				line.clear();
			}
		}

		return nextFreeId;
	}



	auto ShaderFamily::LogInvalidUniformAccess(std::string_view const name) -> void
	{
		internal::Logger::Instance().Trace(std::format("Ignoring attempt to set shader uniform [{}]: the uniform does not exist in the current permutation.", name));
	}



	auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath) -> ShaderFamily
	{
		auto fileInfo = ReadShaderFiles(std::move(vertexShaderPath), std::move(geometryShaderPath), std::move(fragmentShaderPath));
		auto sourceInfo = ProcessShaderIncludes(std::move(fileInfo));
		return ShaderFamily{sourceInfo};
	}
}
