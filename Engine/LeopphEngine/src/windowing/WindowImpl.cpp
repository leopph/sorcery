#include "windowing/WindowImpl.hpp"

#include "InternalContext.hpp"
#include "Logger.hpp"
#include "SettingsImpl.hpp"
#include "windowing/GlWindow.hpp"

#include <stdexcept>


namespace leopph::internal
{
	std::unique_ptr<WindowImpl> WindowImpl::Create()
	{
		std::unique_ptr<WindowImpl> ret;

		switch (GetSettingsImpl()->GetGraphicsApi())
		{
			case Settings::GraphicsApi::OpenGl:
				ret = std::make_unique<GlWindow>();
		}

		if (!ret)
		{
			auto const errMsg{"Failed to create window: the selected graphics API is not supported."};
			Logger::Instance().Critical(errMsg);
			throw std::runtime_error{errMsg};
		}

		return ret;
	}
}
