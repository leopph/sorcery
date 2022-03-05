#pragma once

#include <memory>


namespace leopph::internal
{
	// Base class for all renderers.
	class Renderer
	{
		public:
			// Invokes Create() on the API-appropriate renderer base class
			static auto Create() -> std::unique_ptr<Renderer>;

			virtual auto Render() -> void = 0;

			Renderer(const Renderer& other) = delete;
			auto operator=(const Renderer& other) -> Renderer& = delete;

			Renderer(Renderer&& other) noexcept = delete;
			auto operator=(Renderer&& other) noexcept -> Renderer& = delete;

			virtual ~Renderer() = default;

		protected:
			Renderer() = default;
	};
}
