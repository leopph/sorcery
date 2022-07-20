#pragma once

#include "Equal.hpp"
#include "Hash.hpp"
#include "Matrix.hpp"
#include "ShaderCommon.hpp"
#include "Types.hpp"
#include "Vector.hpp"

#include <span>
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

			auto Uniform(std::string_view name, bool value) const -> void;
			auto Uniform(std::string_view name, i32 value) const -> void;
			auto Uniform(std::string_view name, u32 value) const -> void;
			auto Uniform(std::string_view name, f32 value) const -> void;
			auto Uniform(std::string_view name, Vector3 const& value) const -> void;
			auto Uniform(std::string_view name, Matrix4 const& value) const -> void;
			auto Uniform(std::string_view name, std::span<i32 const> values) const -> void;
			auto Uniform(std::string_view name, std::span<u32 const> values) const -> void;
			auto Uniform(std::string_view name, std::span<f32 const> values) const -> void;
			auto Uniform(std::string_view name, std::span<Vector3 const> values) const -> void;
			auto Uniform(std::string_view name, std::span<Matrix4 const> values) const -> void;

			auto UseCurrentPermutation() const -> void;

		private:
			// Extracts all shader options from the source file and insert them into out, removes the option specifiers from lines, then returns the number of bits required to store the extracted flags that were not already in out.
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
