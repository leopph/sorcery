#pragma once

#include "Compare.hpp"
#include "Hash.hpp"
#include "Matrix.hpp"
#include "Types.hpp"
#include "Vector.hpp"

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>


namespace leopph
{
	class ShaderProgram
	{
		public:
			explicit ShaderProgram(std::span<std::string> sourceLines);


			void use() const;


			template<typename T>
				requires std::is_fundamental_v<T>
			bool set_uniform(std::string_view name, T value) const;

			template<typename T>
				requires std::is_compound_v<T>
			bool set_uniform(std::string_view name, T const& value) const;

		private:
			template<class T>
			bool set_uniform_internal_forward(std::string_view name, T&& value) const;

			static void set_uniform_internal_concrete(u32 program, i32 location, bool value);
			static void set_uniform_internal_concrete(u32 program, i32 location, i32 value);
			static void set_uniform_internal_concrete(u32 program, i32 location, u32 value);
			static void set_uniform_internal_concrete(u32 program, i32 location, f32 value);
			static void set_uniform_internal_concrete(u32 program, i32 location, Vector3 const& value);
			static void set_uniform_internal_concrete(u32 program, i32 location, Matrix4 const& value);
			static void set_uniform_internal_concrete(u32 program, i32 location, std::span<i32 const> values);
			static void set_uniform_internal_concrete(u32 program, i32 location, std::span<u32 const> values);
			static void set_uniform_internal_concrete(u32 program, i32 location, std::span<f32 const> values);
			static void set_uniform_internal_concrete(u32 program, i32 location, std::span<Vector3 const> values);
			static void set_uniform_internal_concrete(u32 program, i32 location, std::span<Matrix4 const> values);

			static void log_invalid_uniform_access(std::string_view uniformName);


			[[nodiscard]] static std::optional<std::string> compile_shader(u32 shader, std::span<char const* const> lines);
			[[nodiscard]] static std::optional<std::string> link_program(u32 program);


		public:
			ShaderProgram(ShaderProgram const& other) = delete;
			void operator=(ShaderProgram const& other) = delete;

			ShaderProgram(ShaderProgram&& other) = delete;
			void operator=(ShaderProgram&& other) = delete;

			~ShaderProgram();

		private:
			static char const* const VERSION_LINE;
			static char const* const VERTEX_DEFINE_LINE;
			static char const* const GEOMETRY_DEFINE_LINE;
			static char const* const FRAGMENT_DEFINE_LINE;

			u32 mProgram;
			std::unordered_map<std::string, i32, StringHash, StringEqual> mUniformLocations;
	};



	template<typename T>
		requires std::is_fundamental_v<T>
	bool ShaderProgram::set_uniform(std::string_view const name, T value) const
	{
		return set_uniform_internal_forward(name, value);
	}



	template<typename T>
		requires std::is_compound_v<T>
	bool ShaderProgram::set_uniform(std::string_view const name, T const& value) const
	{
		return set_uniform_internal_forward(name, value);
	}



	template<class T>
	bool ShaderProgram::set_uniform_internal_forward(std::string_view const name, T&& value) const
	{
		if (auto const it = mUniformLocations.find(name); it != std::end(mUniformLocations))
		{
			set_uniform_internal_concrete(mProgram, it->second, value);
			return true;
		}

		log_invalid_uniform_access(name);
		return false;
	}
}
