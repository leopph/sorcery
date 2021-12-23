#include "logger.h"

#include <spdlog/spdlog.h>

#include <stdexcept>


namespace leopph::internal
{
	Logger::Logger()
	{
		CurrentLevel(Level::RELEASE);
	}

	auto Logger::Instance() -> Logger&
	{
		static Logger instance;
		return instance;
	}

	auto Logger::CurrentLevel(const Level level) -> void
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

	auto Logger::CurrentLevel() const -> Logger::Level
	{
		switch (spdlog::get_level())
		{
			case spdlog::level::debug:
				return Level::DEBUG;

			case spdlog::level::warn:
				return Level::RELEASE;

			default:
				const auto errMsg{"Internal error: Unknown log level found."};
				spdlog::error(errMsg);
				throw std::domain_error{errMsg};
		}
	}

	auto Logger::Debug(const std::string_view msg) const -> void
	{
		spdlog::debug(msg);
	}

	auto Logger::Critical(const std::string_view msg) const -> void
	{
		spdlog::critical(msg);
	}

	auto Logger::Error(const std::string_view msg) const -> void
	{
		spdlog::error(msg);
	}

	auto Logger::Warning(const std::string_view msg) const -> void
	{
		spdlog::warn(msg);
	}
}
