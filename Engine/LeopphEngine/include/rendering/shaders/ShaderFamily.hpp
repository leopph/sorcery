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

			static bool SetGlobalOption(std::string_view name, u8 value);
			bool SetInstanceOption(std::string_view name, u8 value);

			static bool AddGlobalOption(std::string_view name, u8 min, u8 max);

		private:
			// Sets the global option bits in the current permutation bitset.
			void ApplyGlobalOptions();

			// Sets the instance option bits in the current permutation bitset.
			void ApplyInstanceOptions();

			// Returns the next shift value that can be used to create a new global option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Returns nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] static std::optional<u8> NextFreeGlobalShift(u8 requiredBits);

			// Returns the next shift value that can be used to create a new instance option that requires the passed amount of bits.
			// The returned value represents a left shift.
			// Return nullopt if the required amount of bits could not be shifted into storage without overwriting existing values.
			[[nodiscard]] std::optional<u8> NextFreeInstanceShift(u8 requiredBits) const;

		public:
			bool SetUniform(std::string_view name, bool value);
			bool SetUniform(std::string_view name, i32 value);
			bool SetUniform(std::string_view name, u32 value);
			bool SetUniform(std::string_view name, f32 value);
			bool SetUniform(std::string_view name, Vector3 const& value);
			bool SetUniform(std::string_view name, Matrix4 const& value);
			bool SetUniform(std::string_view name, std::span<i32 const> values);
			bool SetUniform(std::string_view name, std::span<u32 const> values);
			bool SetUniform(std::string_view name, std::span<f32 const> values);
			bool SetUniform(std::string_view name, std::span<Vector3 const> values);
			bool SetUniform(std::string_view name, std::span<Matrix4 const> values);

		private:
			// Logs a message related to accessing a non-existent uniform.
			static void LogInvalidUniformAccess(std::string_view name);

		public:
			bool UseCurrentPermutation();

		private:
			std::optional<Permutation*> GetCurrentPermutation();

			void ExtractInstanceOptions();

			[[nodiscard]] bool CompileCurrentPermutation();
			[[nodiscard]] static std::optional<std::string> CompileShader(u32 shader, std::span<std::string const> lines);
			[[nodiscard]] static std::optional<std::string> LinkProgram(u32 program);

			// Queries the uniform locations from the permutation's program and fills its cache with the values.
			static void QueryUniformLocations(Permutation& perm);

		public:
			ShaderFamily(ShaderFamily const& other) = delete;
			void operator=(ShaderFamily const& other) = delete;

			ShaderFamily(ShaderFamily&& other) = delete;
			void operator=(ShaderFamily&& other) = delete;

			~ShaderFamily();

		private:
			static std::vector<ShaderFamily*> s_Instances;

			static std::vector<ShaderOptionInfo> s_GlobalOptions;
			static std::unordered_map<std::string, std::size_t, StringHash, StringEqual> s_GlobalOptionIndexByName;

			std::vector<ShaderOptionInfo> m_InstanceOptions;
			std::unordered_map<std::string, std::size_t, StringHash, StringEqual> m_InstanceOptionIndexByName;

			std::unordered_map<PermutationBitset, Permutation> m_PermutationByBitset;
			PermutationBitset m_CurrentPermutationBitset;

			ShaderProgramSourceInfo m_SourceInfo;
	};



	[[nodiscard]] ShaderFamily MakeShaderFamily(std::filesystem::path vertexShaderPath, std::filesystem::path geometryShaderPath = {}, std::filesystem::path fragmentShaderPath = {});
}
