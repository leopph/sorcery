#include "rendering/shaders/ShaderFamily.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "Util.hpp"
#include "rendering/shaders/ShaderProcessing.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <regex>


namespace leopph
{
	ShaderFamily::ShaderFamily(ShaderProgramSourceInfo sourceInfo) :
		mCurrentPermutationBitset{0},
		mSourceInfo{std::move(sourceInfo)}
	{
		extract_instance_options();

		auto const AddNewLineChars = [](std::vector<std::string>& lines)
		{
			for (auto& line : lines)
			{
				line.push_back('\n');
			}
		};

		AddNewLineChars(mSourceInfo.vertex);

		if (mSourceInfo.geometry)
		{
			AddNewLineChars(*mSourceInfo.geometry);
		}

		if (mSourceInfo.fragment)
		{
			AddNewLineChars(*mSourceInfo.fragment);
		}

		sInstances.push_back(this);

		/*// A concrete state of an option
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
		permutationInstances.emplace_back(mCurrentPermutationBitset, defaultOptInstances);

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
			mPermutationByBitset.emplace(std::move(permInstance.bitset), std::move(perm));
		}*/
	}



	bool ShaderFamily::set_global_option(std::string_view const name, u8 const value)
	{
		auto const it = sGlobalOptionIndexByName.find(name);

		if (it == std::end(sGlobalOptionIndexByName))
		{
			internal::Logger::Instance().Debug(std::format("Ignoring attempt to set global shader option [{}]: the option does not exist.", name));
			return false;
		}

		auto& option = s_GlobalOptions[it->second];

		if (option.min > value || option.max < value)
		{
			internal::Logger::Instance().Error(std::format("Error setting global shader option value: value [{}] was out of range for option [{}].", value, name));
			return false;
		}

		option.currentValue = value;
		return true;
	}



	bool ShaderFamily::set_instance_option(std::string_view const name, u8 value)
	{
		auto const it = mInstanceOptionIndexByName.find(name);

		if (it == std::end(mInstanceOptionIndexByName))
		{
			internal::Logger::Instance().Debug(std::format("Ignoring attempt to set instance shader option [{}]: the option was not specified in the shader.", name));
			return false;
		}

		auto& option = mInstanceOptions[it->second];

		if (option.min > value || option.max < value)
		{
			internal::Logger::Instance().Error(std::format("Error setting instance shader option value: value [{}] was out of range for option [{}].", value, name));
			return false;
		}

		option.currentValue = value;
		return true;
	}



	bool ShaderFamily::add_global_option(std::string_view const name, u8 const min, u8 const max)
	{
		auto const maxValueNorm = max - min;
		auto const requiredBits = math::BinaryDigitCount(maxValueNorm);

		if (auto const nextShift = next_free_global_shift(requiredBits))
		{
			PermutationBitset const mask = ((1 << requiredBits) - 1) << *nextShift;
			s_GlobalOptions.emplace_back(std::string{name}, mask, *nextShift, requiredBits, min, max, min);
			sGlobalOptionIndexByName[std::string{name}] = s_GlobalOptions.size() - 1;

			return true;
		}

		return false;
	}



	void ShaderFamily::apply_global_options()
	{
		for (auto const& option : s_GlobalOptions)
		{
			mCurrentPermutationBitset = (mCurrentPermutationBitset & ~option.mask) | ((option.currentValue - option.min) << option.shift);
		}
	}



	void ShaderFamily::apply_instance_options()
	{
		for (auto const& option : mInstanceOptions)
		{
			mCurrentPermutationBitset = (mCurrentPermutationBitset & ~option.mask) | ((option.currentValue - option.min) << option.shift);
		}
	}



	std::optional<u8> ShaderFamily::next_free_global_shift(u8 const requiredBits)
	{
		auto nextShift = std::numeric_limits<PermutationBitset>::digits;

		for (auto const& option : s_GlobalOptions)
		{
			nextShift = std::min<decltype(nextShift)>(nextShift, option.shift);
		}

		if (nextShift - requiredBits >= 0)
		{
			return clamp_cast<u8>(nextShift - requiredBits);
		}

		return std::nullopt;
	}



	std::optional<u8> ShaderFamily::next_free_instance_shift(u8 const requiredBits) const
	{
		auto nextShift{0};

		for (auto const& option : mInstanceOptions)
		{
			nextShift = std::max(option.shift + - option.numBits, nextShift);
		}

		if (nextShift + requiredBits < std::numeric_limits<PermutationBitset>::digits)
		{
			return clamp_cast<u8>(nextShift);
		}

		return std::nullopt;
	}



	ShaderProgram* ShaderFamily::get_current_permutation()
	{
		apply_global_options();
		apply_instance_options();

		auto permIt = mPermutationByBitset.find(mCurrentPermutationBitset);

		if (permIt == std::end(mPermutationByBitset))
		{
			if (!compile_current_permutation())
			{
				internal::Logger::Instance().Error("Error accessing shader permutation: the shader permutation does not exist.");
				return nullptr;
			}

			permIt = mPermutationByBitset.find(mCurrentPermutationBitset);
		}

		return &permIt->second;
	}



	void ShaderFamily::extract_instance_options()
	{
		std::regex const static validFullOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s+min\s*=\s*\d+\s+max\s*=\s*\d+\s*)delim"};
		std::regex const static validShortOptionRegex{R"delim(^\s*#\s*pragma\s+option\s+name\s*=\s*[A-Za-z][\S]*\s*)delim"};
		std::regex const static nameAssignment{R"delim(=\s*[A-Za-z][\S]*)delim"};
		std::regex const static numberAssignment{R"delim(=\s*\d+)delim"};
		std::regex const static nameIdentifier{R"delim([A-Za-z][\S]*)delim"};
		std::regex const static number{R"delim(\d+)delim"};

		auto const ExtractFrom = [this](std::vector<std::string>& lines) -> void
		{
			for (auto& line : lines)
			{
				if (std::regex_search(line, validFullOptionRegex))
				{
					std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
					auto const assignmentStr = nameAssignmentMatch->str();
					std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};

					std::regex_iterator numberAssignmentMatch{std::begin(line), std::end(line), numberAssignment};
					auto const minAssignmentStr = numberAssignmentMatch->str();
					std::regex_iterator minMatch{std::begin(minAssignmentStr), std::end(minAssignmentStr), number};

					++numberAssignmentMatch;
					auto const maxAssignmentStr = numberAssignmentMatch->str();
					std::regex_iterator maxMatch{std::begin(maxAssignmentStr), std::end(maxAssignmentStr), number};

					auto const min = clamp_cast<u8>(std::stoi(minMatch->str()));
					auto const max = clamp_cast<u8>(std::stoi(maxMatch->str()));

					// Temporary mask and shift, will calculate it after eliminating duplicates.
					mInstanceOptions.emplace_back(nameMatch->str(), static_cast<u8>(0), static_cast<u8>(0), math::BinaryDigitCount(max - min), min, max, min);
					line.clear();
				}
				else if (std::regex_search(line, validShortOptionRegex))
				{
					std::regex_iterator nameAssignmentMatch{std::begin(line), std::end(line), nameAssignment};
					auto const assignmentStr = nameAssignmentMatch->str();
					std::regex_iterator nameMatch{std::begin(assignmentStr), std::end(assignmentStr), nameIdentifier};

					// Temporary mask and shift, will calculate it after eliminating duplicates.
					mInstanceOptions.emplace_back(nameMatch->str(), static_cast<u8>(0), static_cast<u8>(0), static_cast<u8>(1), static_cast<u8>(0), static_cast<u8>(1), static_cast<u8>(0));
					line.clear();
				}
			}
		};

		ExtractFrom(mSourceInfo.vertex);

		if (mSourceInfo.geometry)
		{
			ExtractFrom(*mSourceInfo.geometry);
		}

		if (mSourceInfo.fragment)
		{
			ExtractFrom(*mSourceInfo.fragment);
		}

		for (std::size_t i = 0; i < mInstanceOptions.size(); i++)
		{
			for (auto j = i + 1; j < mInstanceOptions.size();)
			{
				if (mInstanceOptions[i].name == mInstanceOptions[j].name)
				{
					if (mInstanceOptions[i].min != mInstanceOptions[j].min || mInstanceOptions[i].max != mInstanceOptions[j].max)
					{
						internal::Logger::Instance().Warning(std::format("Ignoring shader option [{}] while extracting because it was already specified with a different value set.", mInstanceOptions[i].name));
					}

					mInstanceOptions.erase(std::begin(mInstanceOptions) + j);
				}
				else
				{
					j++;
				}
			}
		}

		u8 nextShift{0};

		for (auto& option : mInstanceOptions)
		{
			option.mask = ((1 << option.numBits) - 1) << nextShift;
			option.shift = nextShift;
			nextShift += option.numBits;
		}
	}



	bool ShaderFamily::compile_current_permutation()
	{
		u32 usedLocalBits{0};

		for (auto const& instanceOption : mInstanceOptions)
		{
			usedLocalBits = clamp_cast<u32>(std::max<i64>(usedLocalBits, instanceOption.numBits + instanceOption.shift));
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

		for (auto const& options : {std::cref(mInstanceOptions), std::cref(s_GlobalOptions)})
		{
			for (auto const& option : options.get())
			{
				optionDefines.push_back(std::format("#define {} {}\n", option.name, option.currentValue));
			}
		}

		auto sourceInfo = mSourceInfo;
		std::string versionLine = "#version 450 core\n";

		if (sourceInfo.fragment)
		{
			sourceInfo.fragment->insert(std::begin(*sourceInfo.fragment), std::begin(optionDefines), std::end(optionDefines));
			sourceInfo.fragment->insert(std::begin(*sourceInfo.fragment), versionLine);
		}

		if (sourceInfo.geometry)
		{
			sourceInfo.geometry->insert(std::begin(*sourceInfo.geometry), std::begin(optionDefines), std::end(optionDefines));
			sourceInfo.geometry->insert(std::begin(*sourceInfo.geometry), versionLine);
		}

		sourceInfo.vertex.insert(std::begin(sourceInfo.vertex), std::make_move_iterator(std::begin(optionDefines)), std::make_move_iterator(std::end(optionDefines)));
		sourceInfo.vertex.insert(std::begin(sourceInfo.vertex), std::move(versionLine));

		mPermutationByBitset.emplace(mCurrentPermutationBitset, sourceInfo);

		return true;
	}



	ShaderFamily::~ShaderFamily()
	{
		std::erase(sInstances, this);
	}



	std::vector<gsl::not_null<ShaderFamily*>> ShaderFamily::sInstances;
	std::vector<ShaderFamily::ShaderOptionInfo> ShaderFamily::s_GlobalOptions;
	std::unordered_map<std::string, std::size_t, StringHash, StringEqual> ShaderFamily::sGlobalOptionIndexByName;



	ShaderFamily make_shader_family(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath, std::filesystem::path fragmentShaderPath)
	{
		auto fileInfo = ReadShaderFiles(std::move(vertexShaderPath), std::move(geometryShaderPath), std::move(fragmentShaderPath));
		auto sourceInfo = ProcessShaderIncludes(std::move(fileInfo));
		return ShaderFamily{std::move(sourceInfo)};
	}
}
