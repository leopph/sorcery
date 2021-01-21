#pragma once

#include "leopph.h"

//#include <memory>

namespace leopph
{
	//std::unique_ptr<App> LEOPPHAPI CreateApp();

	class LEOPPHAPI App
	{
	public:
		App();
		~App();

		App(const App&) = delete;
		App(App&&) = delete;

		virtual App& operator=(const App&) = delete;
		virtual App& operator=(App&&) = delete;

		virtual void Run() = 0;
	};
}