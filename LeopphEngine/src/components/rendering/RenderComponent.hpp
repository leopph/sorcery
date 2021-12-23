#pragma once

#include "../Component.hpp"
#include "../../api/LeopphApi.hpp"

#include <memory>


namespace leopph::internal
{
	class MeshDataGroup;
	class GlMeshGroup;


	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
		public:
			RenderComponent(const RenderComponent& other) = delete;
			auto operator=(const RenderComponent& other) -> RenderComponent& = delete;

			RenderComponent(RenderComponent&& other) = delete;
			auto operator=(RenderComponent&& other) -> RenderComponent& = delete;

			~RenderComponent() noexcept override;

			/* Get whether the rendered object occludes light from other objects.
			 * This only works if the Light used also has this property set to true.
			 * When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			 * This value is false by default. */
			[[nodiscard]]
			LEOPPHAPI auto CastsShadow() const -> bool;

			/* Set whether the rendered object occludes light from other objects.
			 * This only works if the Light used also has this property set to true.
			 * When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			 * This value is false by default. */
			LEOPPHAPI auto CastsShadow(bool value) -> void;

			/* Get whether the object is rendered together with other objects that use the same data source.
			 * This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			 * The default value is false. */
			[[nodiscard]]
			LEOPPHAPI auto Instanced() const -> bool;

			/* Set whether the object is rendered together with other objects that use the same data source.
			 * This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			 * The default value is false. */
			LEOPPHAPI auto Instanced(bool value) -> void;

		protected:
			RenderComponent(leopph::Entity* entity, std::shared_ptr<const MeshDataGroup> meshDataGroup);

		private:
			bool m_CastsShadow{false};
			bool m_Instanced{false};
			std::unique_ptr<GlMeshGroup> m_Renderable{nullptr};
	};
}
