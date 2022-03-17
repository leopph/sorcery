#pragma once

#include "RenderComponent.hpp"
#include "../../api/LeopphApi.hpp"
#include "../../rendering/geometry/MeshDataGroup.hpp"

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
			auto GetMeshData() -> std::shared_ptr<const internal::MeshDataGroup>;

			const static std::string s_MeshDataId;


			class CubeMeshDataGroup final : public internal::MeshDataGroup
			{
				public:
					CubeMeshDataGroup();

				private:
					static std::weak_ptr<Material> s_Material;
			};
	};
}
