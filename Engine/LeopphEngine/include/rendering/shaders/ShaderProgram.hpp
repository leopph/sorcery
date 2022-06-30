#pragma once

#include "Matrix.hpp"
#include "ShaderStageInfo.hpp"
#include "Vector.hpp"
#include "util/equal/StringEqual.hpp"
#include "util/hash/StringHash.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>


namespace leopph::internal
{
	class ShaderProgram
	{
		public:
			explicit ShaderProgram(std::vector<ShaderStageInfo> const& stageInfo);
			explicit ShaderProgram(std::span<unsigned char const> binary);

			ShaderProgram(ShaderProgram const& other) = delete;
			auto operator=(ShaderProgram const& other) -> ShaderProgram& = delete;

			ShaderProgram(ShaderProgram&& other) = delete;
			auto operator=(ShaderProgram&& other) -> ShaderProgram& = delete;

			~ShaderProgram() noexcept;

			auto Use() const -> void;

			auto SetUniform(std::string_view name, bool value) -> void;
			auto SetUniform(std::string_view name, int value) -> void;
			auto SetUniform(std::string_view name, unsigned value) -> void;
			auto SetUniform(std::string_view name, float value) -> void;
			auto SetUniform(std::string_view name, Vector3 const& value) -> void;
			auto SetUniform(std::string_view name, Matrix4 const& value) -> void;
			auto SetUniform(std::string_view name, std::span<int const> values) -> void;
			auto SetUniform(std::string_view name, std::span<unsigned const> values) -> void;
			auto SetUniform(std::string_view name, std::span<float const> values) -> void;
			auto SetUniform(std::string_view name, std::span<Vector3 const> values) -> void;
			auto SetUniform(std::string_view name, std::span<Matrix4 const> values) -> void;

			// Queries the binary representation of the linked program
			[[nodiscard]] auto Binary() const -> std::vector<unsigned char>;

		private:
			[[nodiscard]] static auto CompilationStatus(unsigned name) -> std::pair<bool, std::optional<std::string>>;
			[[nodiscard]] auto LinkStatus() const -> std::pair<bool, std::optional<std::string>>;
			[[nodiscard]] auto GetUniformLocation(std::string_view) -> int;

			std::unordered_map<std::string, int, StringHash, StringEqual> m_UniformLocations;
			unsigned m_ProgramName;
	};
}
