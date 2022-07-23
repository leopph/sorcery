#include "WindowImpl.hpp"

#include "Logger.hpp"
#include "../InternalContext.hpp"
#include "../SettingsImpl.hpp"
#include "../windowing/GlfwWindowImpl.hpp"

#include <stdexcept>


namespace leopph::internal
{
	std::unique_ptr<WindowImpl> WindowImpl::Create()
	{
		std::unique_ptr<WindowImpl> ret;

		switch (GetSettingsImpl()->GetGraphicsApi())
		{
			case Settings::GraphicsApi::OpenGl:
				ret = std::make_unique<GlfwWindowImpl>();
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
