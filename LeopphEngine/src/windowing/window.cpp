#include "window.h"
#include "../config/settings.h"
#include "glwindow.h"
#include "../components/Camera.hpp"

namespace leopph::impl
{
	Window* Window::s_Instance{ nullptr };


	
	Window& Window::Get(unsigned width, unsigned height,
		const std::string& title, bool fullscreen)
	{
		if (s_Instance == nullptr)
		{
			switch (Settings::RenderAPI)
			{
			case Settings::GraphicsAPI::OpenGL:
				s_Instance = new GLWindowImpl{ width, height, title, fullscreen };
			};

			if (Camera::Active())
				Camera::Active()->AspectRatio(s_Instance->m_Width, s_Instance->m_Height);

			s_Instance->InitKeys();
		}

		return *s_Instance;
	}

	void Window::Destroy()
	{
		delete s_Instance;
	}


	
	Window::Window(unsigned width, unsigned height,
		const std::string& title, bool fullscreen) :
		m_Width{ width }, m_Height{ height },
		m_Title{ title }, m_Fullscreen{ fullscreen }
	{}

	

	unsigned Window::Width() const
	{
		return m_Width;
	}

	void Window::Width(unsigned newWidth)
	{
		m_Width = newWidth;
	}

	unsigned Window::Height() const
	{
		return m_Height;
	}

	void Window::Height(unsigned newHeight)
	{
		m_Height = newHeight;
	}

	float Window::AspectRatio() const
	{
		return static_cast<float>(Width()) / Height();
	}

	bool Window::Fullscreen() const
	{
		return m_Fullscreen;
	}

	const leopph::Color& Window::Background() const
	{
		return m_Background;
	}

	void Window::Background(const Color& color)
	{
		m_Background = color;
	}
}