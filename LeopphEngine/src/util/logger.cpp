#include "logger.h"

#include <spdlog/spdlog.h>

#include <stdexcept>

namespace leopph::impl
{
	Logger::Logger()
	{
		CurrentLevel(Level::RELEASE);
	}

	Logger& Logger::Instance()
	{
		static Logger instance;
		return instance;
	}

	void Logger::CurrentLevel(Level level)
	{
		switch (level)
		{
		case Level::DEBUG:
			spdlog::set_level(spdlog::level::debug);
			break;

		case Level::RELEASE:
			spdlog::set_level(spdlog::level::warn);
		}
	}

	Logger::Level Logger::CurrentLevel() const
	{
		switch (spdlog::get_level())
		{
		case spdlog::level::debug:
			return Level::DEBUG;
			break;

		case spdlog::level::warn:
			return Level::RELEASE;

		default:
			const auto errorMsg{"Internal error: Unknown log level found."};
			spdlog::error(errorMsg);
			throw std::runtime_error{ errorMsg };
		}
	}

	void Logger::Debug(std::string_view msg) const
	{
		spdlog::debug(msg);
	}

	void Logger::Critical(std::string_view msg) const
	{
		spdlog::critical(msg);
	}

	void Logger::Error(std::string_view msg) const
	{
		spdlog::error(msg);
	}

	void Logger::Warning(std::string_view msg) const
	{
		spdlog::warn(msg);
	}
}