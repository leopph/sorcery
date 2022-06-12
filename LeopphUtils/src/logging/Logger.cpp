#include "Logger.hpp"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include <filesystem>
#include <stdexcept>
#include <vector>

// ReSharper disable All
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <Psapi.h>
// ReSharper restore All

namespace leopph::internal
{
	Logger::Logger() :
		m_Logger{
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
				return std::make_shared<spdlog::logger>("LeopphLogger", std::begin(sinks), std::end(sinks));
			}()
		}
	{
		CurrentLevel(Level::Trace);
	}


	auto Logger::Instance() -> Logger&
	{
		static Logger instance;
		return instance;
	}


	auto Logger::CurrentLevel(Level const level) -> void
	{
		try
		{
			m_Logger->set_level(m_TranslateMap.At(level));
		}
		catch (std::out_of_range const&)
		{
			auto const errMsg = "Found unknown value while setting log level.";
			Error(errMsg);
			throw std::domain_error{errMsg};
		}
	}


	auto Logger::CurrentLevel() const -> Level
	{
		try
		{
			return m_TranslateMap.At(m_Logger->level());
		}
		catch (std::out_of_range const&)
		{
			auto const errMsg = "Found unknown value while returning current log level.";
			Error(errMsg);
			throw std::domain_error{errMsg};
		}
	}


	auto Logger::Trace(std::string_view const msg) const -> void
	{
		m_Logger->trace(msg);
	}


	auto Logger::Debug(std::string_view const msg) const -> void
	{
		m_Logger->debug(msg);
	}


	auto Logger::Critical(std::string_view const msg) const -> void
	{
		m_Logger->critical(msg);
	}


	auto Logger::Error(std::string_view const msg) const -> void
	{
		m_Logger->error(msg);
	}


	auto Logger::Warning(std::string_view const msg) const -> void
	{
		m_Logger->warn(msg);
	}


	auto Logger::Info(std::string_view const msg) const -> void
	{
		m_Logger->info(msg);
	}


	Bimap<spdlog::level::level_enum, Logger::Level,
	      #ifdef _DEBUG
	      true
	      #else
	      false
	      #endif
	> Logger::m_TranslateMap
	{
		{spdlog::level::level_enum::trace, Level::Trace},
		{spdlog::level::level_enum::debug, Level::Debug},
		{spdlog::level::level_enum::info, Level::Info},
		{spdlog::level::level_enum::warn, Level::Warning},
		{spdlog::level::level_enum::err, Level::Error},
		{spdlog::level::level_enum::critical, Level::Critical}
	};
}
