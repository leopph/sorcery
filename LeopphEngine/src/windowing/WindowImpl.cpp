#include "WindowImpl.hpp"

#include "GLWindow.hpp"
#include "../config/Settings.hpp"
#include "../util/logger.h"


namespace leopph::internal
{
	auto WindowImpl::Create() -> std::unique_ptr<WindowImpl>
	{
		std::unique_ptr<WindowImpl> ret;

		switch (Settings::Instance().RenderingApi())
		{
			case Settings::GraphicsApi::OpenGl:
				 ret = std::make_unique<GlWindow>();
		}

		ret->InitKeys();
		Logger::Instance().Debug("Window created.");
		return ret;
	}
}
