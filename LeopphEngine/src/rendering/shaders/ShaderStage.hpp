#pragma once

#include "ShaderType.hpp"

#include <optional>
#include <string>
#include <string_view>
#include <vector>



namespace leopph::impl
{
	class ShaderProgram;



	class ShaderStage final
	{
		public:
			ShaderStage(const std::string& src,ShaderType type, const std::vector<std::string>& flags = {});

			ShaderStage(const ShaderStage& other) = delete;
			ShaderStage(ShaderStage&& other) noexcept;

			ShaderStage& operator=(const ShaderStage& other) = delete;
			ShaderStage& operator=(ShaderStage&& other) noexcept;

			~ShaderStage();

			void AttachTo(const ShaderProgram& program) const;


		private:
			[[nodiscard]] std::optional<std::string> PreProcess(const std::string& src, std::vector<std::string> flags) const;
			void Compile(std::string_view src) const;
			[[nodiscard]] std::optional<std::string> CheckForCompilationErrors() const;

			unsigned m_Name;
	};
}
