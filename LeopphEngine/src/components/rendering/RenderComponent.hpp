#pragma once

#include "../Component.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"

#include <memory>


namespace leopph::internal
{
	class GlMeshGroup;


	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
		public:
			// Get whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			[[nodiscard]] LEOPPHAPI
			auto CastsShadow() const noexcept -> bool;

			// Set whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			LEOPPHAPI
			auto CastsShadow(bool value) noexcept -> void;

			// Get whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			[[nodiscard]] LEOPPHAPI
			auto Instanced() const noexcept -> bool;

			// Set whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			LEOPPHAPI
			auto Instanced(bool value) noexcept -> void;

			LEOPPHAPI
			auto Owner(Entity* entity) -> void final;
			using Component::Owner;

			LEOPPHAPI
			auto Active(bool active) -> void final;
			using Component::Active;

			RenderComponent(RenderComponent const& other) = default;
			LEOPPHAPI
			auto operator=(RenderComponent const& other) -> RenderComponent&;

			RenderComponent(RenderComponent&& other) = delete;
			auto operator=(RenderComponent&& other) -> RenderComponent& = delete;

			LEOPPHAPI ~RenderComponent() noexcept override;

		protected:
			explicit RenderComponent(std::shared_ptr<MeshGroup const>&& meshGroup);

		private:
			bool m_CastsShadow{false};
			bool m_Instanced{false};
			std::shared_ptr<GlMeshGroup> m_Renderable;
	};
}
