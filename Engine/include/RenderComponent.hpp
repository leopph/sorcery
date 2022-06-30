#pragma once

#include "Component.hpp"
#include "MeshGroup.hpp"

#include <memory>


namespace leopph::internal
{
	class RenderObject;


	// The RenderComponent class provides a base for all Components related to rendering objects.
	class RenderComponent : public Component
	{
		public:
			// Get whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			[[nodiscard]] auto LEOPPHAPI CastsShadow() const noexcept -> bool;

			// Set whether the rendered object occludes light from other objects.
			// This only works if the Light used also has this property set to true.
			// When instancing is turned on if any of the instances have this property set to true, all instances will cast shadow regardless of what is set on them.
			// This value is false by default.
			auto LEOPPHAPI CastsShadow(bool value) noexcept -> void;

			// Get whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			[[nodiscard]] auto LEOPPHAPI Instanced() const noexcept -> bool;

			// Set whether the object is rendered together with other objects that use the same data source.
			// This speeds up rendering but limits the amount of customization that can be applied e.g. shadow casting.
			// The default value is false.
			auto LEOPPHAPI Instanced(bool value) noexcept -> void;

			using Component::Owner;

			using Component::Active;

			LEOPPHAPI ~RenderComponent() override;

		protected:
			RenderComponent() noexcept = default;

			// Deferred initialization.
			// Must always be called before any other function.
			void Init(MeshGroup const& meshGroup) noexcept;

			LEOPPHAPI RenderComponent(RenderComponent const& other) noexcept;
			auto LEOPPHAPI operator=(RenderComponent const& other) noexcept -> RenderComponent&;

			LEOPPHAPI RenderComponent(RenderComponent&& other) noexcept;
			auto LEOPPHAPI operator=(RenderComponent&& other) noexcept -> RenderComponent&;

		private:
			bool m_CastsShadow{false};
			bool m_Instanced{false};
			RenderObject* m_RenderObject; // NOT NULL
	};
}
