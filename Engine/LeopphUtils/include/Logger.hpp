#pragma once

#include "Bimap.hpp"
#include "LeopphApi.hpp"

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


			LEOPPHAPI
			static auto Instance() -> Logger&;

			LEOPPHAPI
			auto CurrentLevel(Level level) -> void;

			[[nodiscard]] LEOPPHAPI
			auto CurrentLevel() const -> Level;

			LEOPPHAPI
			auto Trace(std::string_view msg) const -> void;

			LEOPPHAPI
			auto Debug(std::string_view msg) const -> void;

			LEOPPHAPI
			auto Critical(std::string_view msg) const -> void;

			LEOPPHAPI
			auto Error(std::string_view msg) const -> void;

			LEOPPHAPI
			auto Warning(std::string_view msg) const -> void;

			LEOPPHAPI
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
