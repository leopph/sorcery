#pragma once

#include "Component.hpp"
#include "StaticMesh.hpp"

#include <memory>


namespace leopph::internal
{
	class StaticMeshComponent : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);


		protected:
			void init(std::shared_ptr<StaticMesh> staticMeshGroup);

			StaticMeshComponent() = default;

			LEOPPHAPI StaticMeshComponent(StaticMeshComponent const& other);
			LEOPPHAPI StaticMeshComponent& operator=(StaticMeshComponent const& other);

			LEOPPHAPI StaticMeshComponent(StaticMeshComponent&& other) noexcept;
			LEOPPHAPI StaticMeshComponent& operator=(StaticMeshComponent&& other) noexcept;

		public:
			LEOPPHAPI ~StaticMeshComponent() override;

		private:
			bool mIsCastingShadow{true};
			std::shared_ptr<StaticMesh> mMesh;
	};
}
