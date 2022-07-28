#pragma once

#include "Component.hpp"
#include "StaticMeshGroup.hpp"

#include <memory>


namespace leopph::internal
{
	class StaticMeshGroupComponent : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);


		protected:
			explicit StaticMeshGroupComponent(std::shared_ptr<StaticMeshGroup> staticMeshGroup);

			LEOPPHAPI StaticMeshGroupComponent(StaticMeshGroupComponent const& other);
			LEOPPHAPI StaticMeshGroupComponent& operator=(StaticMeshGroupComponent const& other);

			LEOPPHAPI StaticMeshGroupComponent(StaticMeshGroupComponent&& other) noexcept;
			LEOPPHAPI StaticMeshGroupComponent& operator=(StaticMeshGroupComponent&& other) noexcept;

		public:
			LEOPPHAPI ~StaticMeshGroupComponent() override;

		private:
			bool mIsCastingShadow{true};
			std::shared_ptr<StaticMeshGroup> mMeshGroup;
	};
}
