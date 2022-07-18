#pragma once

#include "Matrix.hpp"
#include "ShaderCommon.hpp"
#include "Types.hpp"
#include "Vector.hpp"
#include "util/equal/StringEqual.hpp"
#include "util/hash/StringHash.hpp"

#include <span>
#include <string>
#include <unordered_map>


namespace leopph::internal
{
	class ShaderProgram2
	{
		public:
			explicit ShaderProgram2(ShaderProgramSourceInfo const& sourceInfo);
			explicit ShaderProgram2(ShaderProgramBinaryInfo const& binaryInfo);
			explicit ShaderProgram2(ShaderProgramCachedBinaryInputInfo const& cachedBinaryInfo);

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

			[[nodiscard]] auto Binary() const -> ShaderProgramCachedBinaryOutputInfo;

		private:
			auto QueryUniformLocations() -> void;

		public:
			ShaderProgram2(ShaderProgram2 const& other) = delete;
			auto operator=(ShaderProgram2 const& other) -> void = delete;

			ShaderProgram2(ShaderProgram2&& other) = delete;
			auto operator=(ShaderProgram2&& other) -> void = delete;

			~ShaderProgram2() noexcept;

		private:
			u32 m_Program;
			u32 m_VertexShader{0};
			u32 m_GeometryShader{0};
			u32 m_FragmentShader{0};
			std::unordered_map<std::string, i32, StringHash, StringEqual> m_UniformLocations;
	};
}
