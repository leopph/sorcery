#pragma once

#include "MeshGroup.hpp"
#include "rendering/RenderObject.hpp"

#include <memory>


namespace leopph::internal
{
	class Renderer
	{
		public:
			static auto Create() -> std::unique_ptr<Renderer>;

			virtual auto Render() -> void = 0;

			[[nodiscard]] virtual auto CreateRenderObject(MeshGroup const& meshGroup) -> RenderObject* = 0;
			virtual auto DeleteRenderObject(RenderObject* renderObject) -> void = 0;

			virtual ~Renderer() noexcept = default;

		protected:
			Renderer() = default;

			Renderer(Renderer const& other) = default;
			auto operator=(Renderer const& other) -> Renderer& = default;

			Renderer(Renderer&& other) noexcept = default;
			auto operator=(Renderer&& other) noexcept -> Renderer& = default;
	};
}
