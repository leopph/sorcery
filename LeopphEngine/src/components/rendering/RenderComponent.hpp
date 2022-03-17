#pragma once

#include "../Component.hpp"

#include <memory>


namespace leopph::internal
{
	class MeshDataGroup;
	class GlMeshGroup;


	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
		public:
			// Get whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			[[nodiscard]] constexpr auto CastsShadow() const noexcept;

			// Set whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			constexpr auto CastsShadow(bool value) noexcept;

			// Get whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			[[nodiscard]] constexpr auto Instanced() const noexcept;

			// Set whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			constexpr auto Instanced(bool value) noexcept;

			// Activate the RenderComponent.
			// Only active, attached RenderComponents are rendered.
			LEOPPHAPI auto Activate() -> void override;

			// Deactivate the RenderComponent.
			// Only active, attached RenderComponents are rendered.
			LEOPPHAPI auto Deactivate() -> void override;

			// Attach the RenderComponent to the Entity.
			// Only active, attached RenderComponents are rendered.
			LEOPPHAPI auto Attach(leopph::Entity* entity) -> void override;

			// Detach the RenderComponent from the Entity.
			// Only active, attached RenderComponents are rendered.
			LEOPPHAPI auto Detach() -> void override;

			RenderComponent(const RenderComponent& other) = delete;
			auto operator=(const RenderComponent& other) -> RenderComponent& = delete;

			RenderComponent(RenderComponent&& other) = delete;
			auto operator=(RenderComponent&& other) -> RenderComponent& = delete;

			~RenderComponent() noexcept override;

		protected:
			explicit RenderComponent(std::shared_ptr<const MeshDataGroup> meshDataGroup);

		private:
			bool m_CastsShadow{false};
			bool m_Instanced{false};
			std::shared_ptr<GlMeshGroup> m_Renderable;
	};


	constexpr auto RenderComponent::CastsShadow() const noexcept
	{
		return m_CastsShadow;
	}


	constexpr auto RenderComponent::CastsShadow(const bool value) noexcept
	{
		m_CastsShadow = value;
	}


	constexpr auto RenderComponent::Instanced() const noexcept
	{
		return m_Instanced;
	}


	constexpr auto RenderComponent::Instanced(const bool value) noexcept
	{
		m_Instanced = value;
	}
}
