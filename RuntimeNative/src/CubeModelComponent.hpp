#pragma once

#include "Component.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

namespace leopph {
	class CubeModelComponent : public Component {
		std::shared_ptr<Material> mMat;
		std::shared_ptr<Mesh> mMesh;

	public:
		LEOPPHAPI CubeModelComponent();
		~CubeModelComponent() override;

		LEOPPHAPI [[nodiscard]] auto GetMaterial() const noexcept -> std::shared_ptr<Material>;
		LEOPPHAPI auto SetMaterial(std::shared_ptr<Material> material) noexcept -> void;

		LEOPPHAPI [[nodiscard]] auto GetMesh() const noexcept -> std::shared_ptr<Mesh>;
		LEOPPHAPI auto SetMesh(std::shared_ptr<Mesh> mesh) noexcept -> void;

		LEOPPHAPI auto OnGui() -> void override;

		[[nodiscard]] LEOPPHAPI auto GetSerializationType() const->Type override;
		LEOPPHAPI static Object::Type const SerializationType;

		LEOPPHAPI auto CreateManagedObject() -> void override;
	};
}
