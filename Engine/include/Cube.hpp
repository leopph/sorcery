#pragma once

#include "LeopphApi.hpp"
#include "StaticModelComponent.hpp"


namespace leopph
{
	// A generic untextured 1x1 cube.
	/*class Cube final : public internal::StaticModelComponent
	{
		public:
			LEOPPHAPI Cube();

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

		private:
			[[nodiscard]] static std::shared_ptr<StaticMesh> create_data();
			static std::weak_ptr<StaticMaterial> sMaterial;
	};*/
}
