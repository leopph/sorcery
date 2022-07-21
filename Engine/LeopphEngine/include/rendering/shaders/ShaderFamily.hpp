#pragma once

#include "Equal.hpp"
#include "Hash.hpp"
#include "Matrix.hpp"
#include "ShaderCommon.hpp"
#include "Types.hpp"
#include "Vector.hpp"

#include <optional>
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
			std::unordered_map<std::string, i32, StringHash, StringEqual> uniformLocations;
		};

		using PermutationBitset = u32;

		struct ShaderOptionInstanceInfo
		{
			std::string name;
			PermutationBitset mask;
			u8 shift; // left shift
			u8 min; // not normalized
			u8 max; // not normalized
		};

		public:
			explicit ShaderFamily(ShaderProgramSourceInfo sourceInfo);

			auto SelectPermutation(PermutationBitset bitset) -> void;

			auto SetOption(std::string_view name, u8 value) -> void;
			static auto AddGlobalOption(std::string_view name, u8 min, u8 max) -> bool;

		private:
			// Returns the next shift value that can be used to create a new global option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Returns nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] static auto NextFreeGlobalShift(u8 requiredBits) -> std::optional<u8>;

			// Returns the next shift value that can be used to create a new instance option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Return nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] auto NextFreeInstanceShift(u8 requiredBits) const -> std::optional<u8>;

		public:
			auto SetUniform(std::string_view name, bool value) const -> void;
			auto SetUniform(std::string_view name, i32 value) const -> void;
			auto SetUniform(std::string_view name, u32 value) const -> void;
			auto SetUniform(std::string_view name, f32 value) const -> void;
			auto SetUniform(std::string_view name, Vector3 const& value) const -> void;
			auto SetUniform(std::string_view name, Matrix4 const& value) const -> void;
			auto SetUniform(std::string_view name, std::span<i32 const> values) const -> void;
			auto SetUniform(std::string_view name, std::span<u32 const> values) const -> void;
			auto SetUniform(std::string_view name, std::span<f32 const> values) const -> void;
			auto SetUniform(std::string_view name, std::span<Vector3 const> values) const -> void;
			auto SetUniform(std::string_view name, std::span<Matrix4 const> values) const -> void;

		private:
			// Logs a message related to accessing a non-existent uniform.
			static auto LogInvalidUniformAccess(std::string_view name) -> void;

		public:
			auto UseCurrentPermutation() const -> void;

		private:
			[[nodiscard]] auto CompilePermutation(PermutationBitset bitset) -> bool;

			static auto CompileShader(u32 shader, std::span<std::string const> lines) -> std::optional<std::string>;
			static auto LinkProgram(u32 program) -> std::optional<std::string>;

			// Queries the uniform locations from the permutation's program and fills its cache with the values.
			static auto QueryUniformLocations(Permutation& perm) -> void;

		public:
			ShaderFamily(ShaderFamily const& other) = delete;
			auto operator=(ShaderFamily const& other) -> void = delete;

			ShaderFamily(ShaderFamily&& other) = delete;
			auto operator=(ShaderFamily&& other) -> void = delete;

			~ShaderFamily();

		private:
			static std::vector<ShaderOptionInstanceInfo> s_GlobalOptions;

			std::vector<ShaderOptionInstanceInfo> m_InstanceOptions;
			std::unordered_map<std::string, std::size_t, StringHash, StringEqual> m_OptionIndexByName;
			std::unordered_map<PermutationBitset, Permutation> m_PermutationByBits;
			PermutationBitset m_CurrentPermutationBits;
			ShaderProgramSourceInfo m_SourceInfo;
	};


	[[nodiscard]] auto MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {}) -> ShaderFamily;
}
