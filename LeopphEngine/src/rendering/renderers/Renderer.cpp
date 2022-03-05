#include "Renderer.hpp"

#include "OpenGlRenderer.hpp"
#include "../../config/Settings.hpp"
#include "../../util/Logger.hpp"

#include <stdexcept>


namespace leopph::internal
{
	auto Renderer::Create() -> std::unique_ptr<Renderer>
	{
		switch (Settings::GraphicsApi())
		{
			case Settings::GraphicsApi::OpenGl:
				return OpenGlRenderer::Create();
		}

		const auto errMsg{"Failed to create renderer: the selected graphics API is not supported."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}
}
