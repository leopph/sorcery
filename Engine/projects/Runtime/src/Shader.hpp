#pragma once

#include "Compare.hpp"
#include "Hash.hpp"
#include "Math.hpp"
#include "Types.hpp"

#include <filesystem>
#include <optional>
#include <span>
#include <string>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <vector>


namespace leopph
{
	class Shader
	{
		public:
			using RawSourceLines = std::vector<std::string>;
			using ProcessedSourceLines = std::vector<std::string>;
			using ProcessedSourceLineView = std::span<std::string const>;


			struct SourceFileInfo
			{
				std::filesystem::path absolutePath;
				RawSourceLines lines;
			};


			explicit Shader(std::filesystem::path const& path);
			explicit Shader(ProcessedSourceLineView sourceLines);


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


			[[nodiscard]] static SourceFileInfo read_source_file(std::filesystem::path const& path);
			static void process_includes_recursive(std::filesystem::path const& absolutePath, RawSourceLines& lines);
			[[nodiscard]] static ProcessedSourceLines process_includes(SourceFileInfo fileInfo);
			[[nodiscard]] static ProcessedSourceLines add_new_line_chars(ProcessedSourceLines sourceLines);


			[[nodiscard]] static std::optional<std::string> compile_shader(u32 shader, std::span<char const* const> lines);
			[[nodiscard]] static std::optional<std::string> link_program(u32 program);


		public:
			Shader(Shader const& other) = delete;
			void operator=(Shader const& other) = delete;

			Shader(Shader&& other) = delete;
			void operator=(Shader&& other) = delete;

			~Shader();

		private:
			static char const* const VERSION_LINE;
			static char const* const BINDLESS_EXT_LINE;
			static char const* const VERTEX_DEFINE_LINE;
			static char const* const FRAGMENT_DEFINE_LINE;

			u32 mProgram;
			std::unordered_map<std::string, i32, StringHash, StringEqual> mUniformLocations;
	};



	template<typename T>
		requires std::is_fundamental_v<T>
	bool Shader::set_uniform(std::string_view const name, T value) const
	{
		return set_uniform_internal_forward(name, value);
	}



	template<typename T>
		requires std::is_compound_v<T>
	bool Shader::set_uniform(std::string_view const name, T const& value) const
	{
		return set_uniform_internal_forward(name, value);
	}



	template<class T>
	bool Shader::set_uniform_internal_forward(std::string_view const name, T&& value) const
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
