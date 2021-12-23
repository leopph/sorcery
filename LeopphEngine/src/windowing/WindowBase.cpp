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


	WindowBase& WindowBase::Get(const unsigned width, const unsigned height,
	                    const std::string& title, const bool fullscreen)
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


	void WindowBase::Destroy()
	{
		delete s_Instance;
		Logger::Instance().Debug("Window destroyed.");
	}


	float WindowBase::AspectRatio() const
	{
		return static_cast<float>(Width()) / static_cast<float>(Height());
	}
}
