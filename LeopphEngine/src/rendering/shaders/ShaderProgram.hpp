#pragma once

#include "ShaderStageInfo.hpp"
#include "../../math/Matrix.hpp"
#include "../../math/Vector.hpp"
#include "../../util/equal/StringEqual.hpp"
#include "../../util/hash/StringHash.hpp"

#include <optional>
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
			explicit ShaderProgram(const std::vector<ShaderStageInfo>& stageInfo);

			ShaderProgram(const ShaderProgram& other) = delete;
			ShaderProgram(ShaderProgram&& other) = delete;

			~ShaderProgram();

			auto operator=(const ShaderProgram& other) -> ShaderProgram& = delete;
			auto operator=(ShaderProgram&& other) -> ShaderProgram& = delete;

			auto Use() const -> void;
			auto Unuse() const -> void;

			auto SetUniform(std::string_view name, bool value) -> void;
			auto SetUniform(std::string_view name, int value) -> void;
			auto SetUniform(std::string_view name, unsigned value) -> void;
			auto SetUniform(std::string_view name, float value) -> void;
			auto SetUniform(std::string_view name, const Vector3& value) -> void;
			auto SetUniform(std::string_view name, const Matrix4& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<bool>& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<int>& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<unsigned>& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<float>& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<Vector3>& value) -> void;
			auto SetUniform(std::string_view name, const std::vector<Matrix4>& value) -> void;

			auto SetBufferBinding(std::string_view bufName, int bindingIndex) -> void;

		private:
			[[nodiscard]] static auto CompilationStatus(unsigned name) -> std::pair<bool, std::optional<std::string>>;
			[[nodiscard]] auto LinkStatus() const -> std::pair<bool, std::optional<std::string>>;
			[[nodiscard]] auto GetUniformLocation(std::string_view) -> int;

			std::unordered_map<std::string, int, StringHash, StringEqual> m_UniformLocations;
			unsigned m_ProgramName;
	};
}
