#pragma once

#include "../containers/Bimap.hpp"

#include <memory>
#include <string_view>

namespace spdlog
{
	class logger;

	namespace level
	{
		enum level_enum : int;
	}
}


namespace leopph::internal
{
	class Logger
	{
		public:
			enum class Level
			{
				Trace, Debug, Info, Warning, Error, Critical
			};


			static auto Instance() -> Logger&;

			auto CurrentLevel(Level level) -> void;

			[[nodiscard]]
			auto CurrentLevel() const -> Level;

			auto Trace(std::string_view msg) const -> void;

			auto Debug(std::string_view msg) const -> void;

			auto Critical(std::string_view msg) const -> void;

			auto Error(std::string_view msg) const -> void;

			auto Warning(std::string_view msg) const -> void;

			auto Info(std::string_view msg) const -> void;

			Logger(Logger const& other) = delete;
			auto operator=(Logger const& other) -> Logger& = delete;

			Logger(Logger&& other) noexcept = delete;
			auto operator=(Logger&& other) noexcept -> Logger& = delete;

		private:
			Logger();
			~Logger() = default;

			std::shared_ptr<spdlog::logger> m_Logger;

			static Bimap<spdlog::level::level_enum, Level,
			             #ifdef _DEBUG
			             true
			             #else
			             false
			             #endif
			> m_TranslateMap;
	};
}
