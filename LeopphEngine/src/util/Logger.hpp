#pragma once

#include <string_view>


namespace leopph::internal
{
	class Logger
	{
		public:
			enum class Level
			{
				Debug, Release
			};


			static auto Instance() -> Logger&;

			auto CurrentLevel(Level level) -> void;
			[[nodiscard]]
			auto CurrentLevel() const -> Level;

			auto Debug(std::string_view msg) const -> void;
			auto Critical(std::string_view msg) const -> void;
			auto Error(std::string_view msg) const -> void;
			auto Warning(std::string_view msg) const -> void;

			Logger(const Logger& other) = delete;
			auto operator=(const Logger& other) -> Logger& = delete;

			Logger(Logger&& other) noexcept = delete;
			auto operator=(Logger&& other) noexcept -> Logger& = delete;

		private:
			Logger();
			~Logger() = default;
	};
}
