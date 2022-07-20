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
			u32 program{0};
			u32 vertex{0};
			u32 geometry{0};
			u32 fragment{0};
			std::unordered_map<std::string, i32, StringHash, StringEqual> uniformLocations;
		};

		struct ShaderOptionInstanceInfo
		{
			u8 index;
			u8 min;
			u8 max;
		};

		public:
			explicit ShaderFamily(ShaderProgramSourceInfo sourceInfo, std::vector<ShaderOptionInfo> options);

			auto Option(std::string_view name, u8 value) -> void;

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
			// Logs a message related to accessing a non-existent uniform.
			static auto LogInvalidUniformAccess(std::string_view name) -> void;

			std::unordered_map<std::string, ShaderOptionInstanceInfo, StringHash, StringEqual> m_OptionsByName;
			std::unordered_map<u32, Permutation> m_PermutationsByBits;
			u32 m_CurrentPermutationBits;
	};


	[[nodiscard]] auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {}) -> ShaderFamily;
}
