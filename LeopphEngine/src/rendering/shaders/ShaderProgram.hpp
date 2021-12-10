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


namespace leopph::impl
{
	class ShaderProgram
	{
		public:
			explicit ShaderProgram(const std::vector<ShaderStageInfo>& stageInfo);

			ShaderProgram(const ShaderProgram& other) = delete;
			ShaderProgram(ShaderProgram&& other) = delete;

			~ShaderProgram();

			ShaderProgram& operator=(const ShaderProgram& other) = delete;
			ShaderProgram& operator=(ShaderProgram&& other) = delete;

			void Use() const;
			void Unuse() const;

			void SetUniform(std::string_view name, bool value);
			void SetUniform(std::string_view name, int value);
			void SetUniform(std::string_view name, unsigned value);
			void SetUniform(std::string_view name, float value);
			void SetUniform(std::string_view name, const Vector3& value);
			void SetUniform(std::string_view name, const Matrix4& value);
			void SetUniform(std::string_view name, const std::vector<bool>& value);
			void SetUniform(std::string_view name, const std::vector<int>& value);
			void SetUniform(std::string_view name, const std::vector<unsigned>& value);
			void SetUniform(std::string_view name, const std::vector<float>& value);
			void SetUniform(std::string_view name, const std::vector<Vector3>& value);
			void SetUniform(std::string_view name, const std::vector<Matrix4>& value);


		private:
			[[nodiscard]] static std::pair<bool, std::optional<std::string>> CompilationStatus(unsigned name);
			[[nodiscard]] std::pair<bool, std::optional<std::string>> LinkStatus() const;
			[[nodiscard]] int GetUniformLocation(std::string_view);

			std::unordered_map<std::string, int, StringHash, StringEqual> m_UniformLocations;
			unsigned m_ProgramName;
	};
}