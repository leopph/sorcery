#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshGroup.hpp"

#include <string>


namespace leopph
{
	// A generic untextured 1x1 cube.
	class Cube final : public internal::RenderComponent
	{
		public:
			LEOPPHAPI Cube();

		private:
			[[nodiscard]] static
			auto GetMeshGroup() -> std::shared_ptr<internal::MeshGroup const>;

			static std::string const s_MeshId;
			static std::weak_ptr<Material> s_Material;
	};
}
