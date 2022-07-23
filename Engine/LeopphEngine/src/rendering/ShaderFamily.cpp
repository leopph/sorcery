#include "ShaderFamily.hpp"

#include "Logger.hpp"
#include "Math.hpp"
#include "ShaderProcessing.hpp"
#include "Util.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iterator>
#include <limits>
#include <optional>
#include <ranges>
#include <regex>
#include <tuple>


namespace leopph
{
	ShaderFamily::ShaderFamily(ShaderProgramSourceLines sourceLines) :
		mCurrentPermutationBitset{0},
		mSourceLines{std::move(sourceLines)}
	{
		// Register ourselves
		sInstances.push_back(this);

		// Make sure there is a newline at the end of every line
		for (auto& line : mSourceLines)
		{
			line.push_back('\n');
		}

		extract_instance_options();


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

			for (auto const& permInfo : permutationInstances)*/
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

		// Extract all found options

		for (auto& line : mSourceLines)
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

		// Deduplicate options

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

		// Calculate mask and shift for the options

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

		mPermutationByBitset.emplace(mCurrentPermutationBitset, mSourceLines);

		return true;
	}



	ShaderFamily::~ShaderFamily()
	{
		std::erase(sInstances, this);
	}



	std::vector<gsl::not_null<ShaderFamily*>> ShaderFamily::sInstances;
	std::vector<ShaderFamily::ShaderOptionInfo> ShaderFamily::s_GlobalOptions;
	std::unordered_map<std::string, std::size_t, StringHash, StringEqual> ShaderFamily::sGlobalOptionIndexByName;



	ShaderFamily make_shader_family(std::filesystem::path const& filePath)
	{
		ShaderProgramSourceFileInfo sourceFileInfo;
		// explicitly ignore error so that we can return a ShaderFamily instance, even if it is empty.
		std::ignore = read_shader_files(filePath, sourceFileInfo);

		ShaderProgramSourceLines sourceLines;
		process_shader_includes(std::move(sourceFileInfo), sourceLines);

		return ShaderFamily{std::move(sourceLines)};
	}
}
