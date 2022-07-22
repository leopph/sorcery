#pragma once

#include "Equal.hpp"
#include "Hash.hpp"
#include "ShaderCommon.hpp"
#include "ShaderProgram.hpp"
#include "Types.hpp"

#include <gsl/pointers>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>



namespace leopph
{
	class ShaderFamily
	{
		using PermutationBitset = u32;


		struct ShaderOptionInfo
		{
			std::string name;
			PermutationBitset mask;
			u8 shift; // left shift
			u8 numBits;
			u8 min; // not normalized
			u8 max; // not normalized
			u8 currentValue; // always set to min on construction
		};


		public:
			explicit ShaderFamily(ShaderProgramSourceInfo sourceInfo);

			static bool set_global_option(std::string_view name, u8 value);
			bool set_instance_option(std::string_view name, u8 value);

			static bool add_global_option(std::string_view name, u8 min, u8 max);

		private:
			// Sets the global option bits in the current permutation bitset.
			void apply_global_options();

			// Sets the instance option bits in the current permutation bitset.
			void apply_instance_options();

			// Returns the next shift value that can be used to create a new global option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Returns nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] static std::optional<u8> next_free_global_shift(u8 requiredBits);

			// Returns the next shift value that can be used to create a new instance option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Return nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] std::optional<u8> next_free_instance_shift(u8 requiredBits) const;

		public:
			ShaderProgram* get_current_permutation();

		private:
			void extract_instance_options();

			[[nodiscard]] bool compile_current_permutation();

		public:
			ShaderFamily(ShaderFamily const& other) = delete;
			void operator=(ShaderFamily const& other) = delete;

			ShaderFamily(ShaderFamily&& other) = delete;
			void operator=(ShaderFamily&& other) = delete;

			~ShaderFamily();

		private:
			static std::vector<gsl::not_null<ShaderFamily*>> sInstances;

			static std::vector<ShaderOptionInfo> s_GlobalOptions;
			static std::unordered_map<std::string, std::size_t, StringHash, StringEqual> sGlobalOptionIndexByName;

			std::vector<ShaderOptionInfo> mInstanceOptions;
			std::unordered_map<std::string, std::size_t, StringHash, StringEqual> mInstanceOptionIndexByName;

			std::unordered_map<PermutationBitset, ShaderProgram> mPermutationByBitset;
			PermutationBitset mCurrentPermutationBitset;

			ShaderProgramSourceInfo mSourceInfo;
	};



	[[nodiscard]] ShaderFamily make_shader_family(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {});
}
