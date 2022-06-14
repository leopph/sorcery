#pragma once

#include "LeopphApi.hpp"
#include "RenderComponent.hpp"
#include "MeshGroup.hpp"


namespace leopph
{
	// A generic untextured 1x1 cube.
	class Cube final : public internal::RenderComponent
	{
		public:
			LEOPPHAPI Cube();

			[[nodiscard]] LEOPPHAPI
			auto Clone() const -> ComponentPtr<> override;

		private:
			[[nodiscard]] static
			auto CreateMeshGroup() -> MeshGroup;
			static std::weak_ptr<Material> s_Material;
	};
}