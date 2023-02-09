#pragma once

#include "Component.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

namespace leopph {
class CubeModelComponent : public Component {
	Material* mMat;
	Mesh* mMesh;

public:
	LEOPPHAPI CubeModelComponent();
	~CubeModelComponent() override;

	LEOPPHAPI [[nodiscard]] auto GetMaterial() const noexcept -> Material&;
	LEOPPHAPI auto SetMaterial(Material& material) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetMesh() const noexcept -> Mesh&;
	LEOPPHAPI auto SetMesh(Mesh& mesh) noexcept -> void;

	LEOPPHAPI auto OnGui() -> void override;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Object::Type const SerializationType;

	LEOPPHAPI auto CreateManagedObject() -> void override;
};
}
