#pragma once

#include "LeopphApi.hpp"
#include "StaticMeshGroupComponent.hpp"


namespace leopph
{
	// A generic untextured 1x1 cube.
	class Cube final : public internal::StaticMeshGroupComponent
	{
		public:
			LEOPPHAPI Cube();

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

		private:
			[[nodiscard]] static std::shared_ptr<StaticMeshGroup> create_data();
			static std::weak_ptr<Material> s_Material;
	};
}
