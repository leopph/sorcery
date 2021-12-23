#pragma once

#include <string_view>

namespace leopph::internal
{
	class Logger
	{
	public:
		enum class Level
		{
			DEBUG, RELEASE
		};

		static Logger& Instance();

		void CurrentLevel(Level level);
		Level CurrentLevel() const;
		
		void Debug(std::string_view msg) const;
		void Critical(std::string_view msg) const;
		void Error(std::string_view msg) const;
		void Warning(std::string_view msg) const;

	private:
		Logger();
	};
}