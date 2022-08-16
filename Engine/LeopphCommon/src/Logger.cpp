#include "Logger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <vector>

// ReSharper disable All
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
// ReSharper restore All


namespace leopph
{
	Logger::Logger() :
		mLogger{
			[]
			{
				constexpr auto bufSz{100u};
				constexpr auto defChar{'\0'};
				#ifdef UNICODE
				std::wstring s(bufSz, defChar);
				#else
				std::string s(bufSz, defChar);
				#endif
				s.resize(GetModuleFileNameEx(GetCurrentProcess(), nullptr, s.data(), static_cast<DWORD>(s.size())));
				auto const logFilePath = std::filesystem::path{s}.parent_path() / "log.txt";

				std::vector<spdlog::sink_ptr> sinks;
				sinks.push_back(std::make_shared<spdlog::sinks::stdout_color_sink_st>());
				sinks.push_back(std::make_shared<spdlog::sinks::basic_file_sink_st>(logFilePath.string(), DMCOLLATE_TRUE));
				return std::make_unique<spdlog::logger>("LeopphEngine", std::begin(sinks), std::end(sinks));
			}()
		}
	{
		set_level(Level::Trace);
	}



	Logger& Logger::get_instance()
	{
		static Logger instance;
		return instance;
	}



	Logger::Level Logger::get_level() const
	{
		return sLevelTranslateMap.at(mLogger->level());
	}



	void Logger::set_level(Level const level) const
	{
		mLogger->set_level(sLevelTranslateMap.at(level));
	}



	void Logger::trace(std::string_view const msg) const
	{
		mLogger->trace(msg);
	}



	void Logger::debug(std::string_view const msg) const
	{
		mLogger->debug(msg);
	}



	void Logger::info(std::string_view const msg) const
	{
		mLogger->info(msg);
	}



	void Logger::warning(std::string_view const msg) const
	{
		mLogger->warn(msg);
	}



	void Logger::error(std::string_view const msg) const
	{
		mLogger->error(msg);
	}



	void Logger::critical(std::string_view const msg) const
	{
		mLogger->critical(msg);
	}



	Bimap<spdlog::level::level_enum, Logger::Level> Logger::sLevelTranslateMap
	{
		{spdlog::level::level_enum::trace, Level::Trace},
		{spdlog::level::level_enum::debug, Level::Debug},
		{spdlog::level::level_enum::info, Level::Info},
		{spdlog::level::level_enum::warn, Level::Warning},
		{spdlog::level::level_enum::err, Level::Error},
		{spdlog::level::level_enum::critical, Level::Critical},
		{spdlog::level::level_enum::off, Level::Off}
	};
}
