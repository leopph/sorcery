#include "Renderer.hpp"

#include "GlRenderer.hpp"
#include "Logger.hpp"
#include "Settings.hpp"

#include <stdexcept>


namespace leopph::internal
{
	auto Renderer::Create() -> std::unique_ptr<Renderer>
	{
		switch (Settings::GraphicsApi())
		{
			case Settings::GraphicsApi::OpenGl:
				return GlRenderer::Create();
		}

		auto const errMsg{"Failed to create renderer: the selected graphics API is not supported."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}
}
