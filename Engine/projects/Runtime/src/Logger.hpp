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


namespace leopph
{
	class Logger
	{
		public:
			enum class Level
			{
				Trace, Debug, Info, Warning, Error, Critical, Off
			};


			[[nodiscard]] LEOPPHAPI static Logger& get_instance();

			[[nodiscard]] LEOPPHAPI Level get_level() const;
			LEOPPHAPI void set_level(Level level) const;

			LEOPPHAPI void trace(std::string_view msg) const;
			LEOPPHAPI void debug(std::string_view msg) const;
			LEOPPHAPI void info(std::string_view msg) const;
			LEOPPHAPI void warning(std::string_view msg) const;
			LEOPPHAPI void error(std::string_view msg) const;
			LEOPPHAPI void critical(std::string_view msg) const;


		private:
			Logger();

		public:
			Logger(Logger const& other) = delete;
			Logger(Logger&& other) = delete;

			Logger& operator=(Logger const& other) = delete;
			Logger& operator=(Logger&& other) = delete;

		private:
			~Logger() = default;

			std::unique_ptr<spdlog::logger> mLogger;

			static Bimap<spdlog::level::level_enum, Level> sLevelTranslateMap;
	};
}
