#pragma once

#include "Equal.hpp"
#include "Hash.hpp"
#include "ShaderCommon.hpp"
#include "Types.hpp"

#include <string>
#include <unordered_map>
#include <vector>

namespace leopph
{
	class ShaderFamily2
	{
		struct Permutation
		{
			u32 program;
			u32 vertex;
			u32 geometry;
			u32 fragment;
			std::unordered_map<std::string, i32, StringHash, StringEqual> uniformLocations;
		};

		struct ShaderOption
		{
			u32 min;
			u32 max;
			u32 id;
		};

		public:
			explicit ShaderFamily2(ShaderProgramSourceInfo const& sourceInfo);
			explicit ShaderFamily2(ShaderProgramSourceFileInfo const& fileInfo);

			auto Option(std::string_view name, u32 value) -> void;

		private:
			// Returns a new string containing the source file with all includes resolved, or an empty string if an error occured.
			static auto ResolveIncludes(ShaderStageSourceFileInfo const& fileInfo) -> std::string;

			// Extracts all shader options from the source file and insert them into out, removes the option specifiers the source string, then returns the number of bits required to store all flags.
			static auto ExtractOptions(std::string& source, std::unordered_map<std::string, ShaderOption, StringHash, StringEqual>& out) -> u32;

			std::unordered_map<std::string, ShaderOption, StringHash, StringEqual> m_Options;
			std::unordered_map<std::vector<bool>, Permutation> m_Permutations;
			std::vector<bool> m_OptionBits;
			std::optional<std::string> m_VertexSource;
			std::optional<std::string> m_GeometrySource;
			std::optional<std::string> m_FragmentSource;

			std::string const static inline s_BuiltInShaderText
			{
				R"delim(
					#version 450 core
					#pragma option name=DIR_SHADOW
					#pragma option name=NUM_DIR_CASCADES min=1 max=4
					#pragma option name=NUM_SPOT         min=0 max=8
					#pragma option name=NUM_SPOT_SHADOW  min=0 max=8
					#pragma option name=NUM_POINT        min=0 max=8
					#pragma option name=NUM_POINT_SHADOW min=0 max=8
				)delim"
			};
	};
}
