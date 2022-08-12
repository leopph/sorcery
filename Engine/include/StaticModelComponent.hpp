#pragma once

#include "Component.hpp"
#include "StaticModelData.hpp"


namespace leopph
{
	class StaticModelComponent : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool value);

			LEOPPHAPI StaticModelComponent(StaticModelData& data);

			LEOPPHAPI StaticModelComponent(StaticModelComponent const& other);
			LEOPPHAPI StaticModelComponent& operator=(StaticModelComponent const& other);

			LEOPPHAPI StaticModelComponent(StaticModelComponent&& other) noexcept;
			LEOPPHAPI StaticModelComponent& operator=(StaticModelComponent&& other) noexcept;

			LEOPPHAPI ~StaticModelComponent() override;

		private:
			bool mIsCastingShadow{true};
	};
}
