#pragma once

#include "Component.hpp"
#include "StaticModelData.hpp"


namespace leopph
{
	class StaticMeshComponent : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);

			LEOPPHAPI explicit StaticMeshComponent(StaticModelData& data);

			LEOPPHAPI StaticMeshComponent(StaticMeshComponent const& other);
			LEOPPHAPI StaticMeshComponent(StaticMeshComponent&& other) noexcept;

			LEOPPHAPI StaticMeshComponent& operator=(StaticMeshComponent const& other);
			LEOPPHAPI StaticMeshComponent& operator=(StaticMeshComponent&& other) noexcept;

			LEOPPHAPI ~StaticMeshComponent() override;

		private:
			bool mIsCastingShadow{true};
	};
}
