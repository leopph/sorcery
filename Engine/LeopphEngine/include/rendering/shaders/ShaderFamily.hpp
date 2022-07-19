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
	class ShaderFamily
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
			explicit ShaderFamily(ShaderProgramSourceInfo const& sourceInfo);

			auto Option(std::string_view name, u32 value) -> void;

		private:
			// Extracts all shader options from the source file and insert them into out, removes the option specifiers from lines, then returns the number of bits required to store all flags.
			static auto ExtractOptions(std::vector<std::string>& sourceLines, std::unordered_map<std::string, ShaderOption, StringHash, StringEqual>& out) -> u32;

			std::unordered_map<std::string, ShaderOption, StringHash, StringEqual> m_Options;
			std::unordered_map<std::vector<bool>, Permutation> m_Permutations;
			std::vector<bool> m_OptionBits;
			std::vector<std::string> m_VertexSource;
			std::optional<std::vector<std::string>> m_GeometrySource;
			std::optional<std::vector<std::string>> m_FragmentSource;
	};


	[[nodiscard]] auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {}) -> ShaderFamily;
}
