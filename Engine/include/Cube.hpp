#pragma once

#include "LeopphApi.hpp"
#include "MeshGroup.hpp"
#include "RenderComponent.hpp"


namespace leopph
{
	// A generic untextured 1x1 cube.
	class Cube final : public internal::RenderComponent
	{
		public:
			LEOPPHAPI Cube();

			[[nodiscard]] LEOPPHAPI ComponentPtr<> Clone() const override;

		private:
			[[nodiscard]] static MeshGroup CreateMeshGroup();
			static std::weak_ptr<Material> s_Material;
	};
}
