#pragma once

#include "LeopphApi.hpp"
#include "StaticMeshComponent.hpp"


namespace leopph
{
	// A generic untextured 1x1 cube.
	class Cube final : public internal::StaticMeshComponent
	{
		public:
			LEOPPHAPI Cube();

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

		private:
			[[nodiscard]] static std::shared_ptr<StaticMesh> create_data();
			static std::weak_ptr<StaticMaterial> sMaterial;
	};
}
