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
			static Logger& Instance();

			LEOPPHAPI void CurrentLevel(Level level);

			[[nodiscard]] LEOPPHAPI Level CurrentLevel() const;

			LEOPPHAPI void Trace(std::string_view msg) const;

			LEOPPHAPI void Debug(std::string_view msg) const;

			LEOPPHAPI void Critical(std::string_view msg) const;

			LEOPPHAPI void Error(std::string_view msg) const;

			LEOPPHAPI void Warning(std::string_view msg) const;

			LEOPPHAPI void Info(std::string_view msg) const;

			Logger(Logger const& other) = delete;
			Logger& operator=(Logger const& other) = delete;

			Logger(Logger&& other) noexcept = delete;
			Logger& operator=(Logger&& other) noexcept = delete;

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
