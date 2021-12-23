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


			static auto Instance() -> Logger&;

			auto CurrentLevel(Level level) -> void;
			auto CurrentLevel() const -> Level;

			auto Debug(std::string_view msg) const -> void;
			auto Critical(std::string_view msg) const -> void;
			auto Error(std::string_view msg) const -> void;
			auto Warning(std::string_view msg) const -> void;

		private:
			Logger();
	};
}
