#pragma once

#include "Component.hpp"
#include "Material.hpp"
#include "Mesh.hpp"

#include <span>
#include <vector>


namespace leopph {
class StaticMeshComponent : public Component {
	std::vector<Material*> mMaterials;
	Mesh* mMesh;

	auto AdjustMaterialListForMesh() -> void;

public:
	LEOPPHAPI StaticMeshComponent();
	~StaticMeshComponent() override;

	[[nodiscard]] LEOPPHAPI auto GetMaterials() const noexcept -> std::span<Material* const>;
	LEOPPHAPI auto SetMaterials(std::vector<Material*> materials) -> void;
	LEOPPHAPI auto ReplaceMaterial(int idx, Material& mtl) -> void;

	[[nodiscard]] LEOPPHAPI auto GetMesh() const noexcept -> Mesh&;
	LEOPPHAPI auto SetMesh(Mesh& mesh) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto CreateManagedObject() -> void override;

	auto Serialize(YAML::Node& node) const -> void override;
	auto Deserialize(YAML::Node const& node) -> void override;

	[[nodiscard]] LEOPPHAPI auto CalculateBounds() const noexcept -> AABB;
};
}
