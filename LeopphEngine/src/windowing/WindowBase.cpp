#include "WindowBase.hpp"

#include "GLWindow.hpp"
#include "../components/Camera.hpp"
#include "../config/Settings.hpp"
#include "../util/logger.h"


namespace leopph::internal
{
	WindowBase* WindowBase::s_Instance{nullptr};

	WindowBase::~WindowBase()
	{
		s_Instance = nullptr;
	}

	auto WindowBase::Get(const unsigned width, const unsigned height,
	                     const std::string& title, const bool fullscreen) -> WindowBase&
	{
		if (s_Instance == nullptr)
		{
			switch (Settings::RenderAPI)
			{
				case Settings::GraphicsApi::OpenGl:
					s_Instance = new GlWindow{static_cast<int>(width), static_cast<int>(height), title, fullscreen};
			}

			s_Instance->InitKeys();

			Logger::Instance().Debug("Window created.");
		}

		return *s_Instance;
	}

	auto WindowBase::Destroy() -> void
	{
		delete s_Instance;
		Logger::Instance().Debug("Window destroyed.");
	}

	auto WindowBase::AspectRatio() const -> float
	{
		return static_cast<float>(Width()) / static_cast<float>(Height());
	}
}
