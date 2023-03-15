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

public:
	LEOPPHAPI StaticMeshComponent();
	~StaticMeshComponent() override;

	LEOPPHAPI [[nodiscard]] auto GetMaterials() const noexcept -> std::span<Material* const>;
	LEOPPHAPI auto AddMaterial(Material& mtl) noexcept -> void;
	LEOPPHAPI auto RemoveMaterial(int idx) noexcept -> void;
	LEOPPHAPI auto ReplaceMaterial(int idx, Material& mtl) noexcept -> void;
	LEOPPHAPI auto SetMaterials(std::vector<Material*> materials) noexcept -> void;

	LEOPPHAPI [[nodiscard]] auto GetMesh() const noexcept -> Mesh&;
	LEOPPHAPI auto SetMesh(Mesh& mesh) noexcept -> void;

	[[nodiscard]] LEOPPHAPI auto GetSerializationType() const -> Type override;
	LEOPPHAPI static Type const SerializationType;

	LEOPPHAPI auto CreateManagedObject() -> void override;

	auto Serialize(YAML::Node& node) const -> void override;
	auto Deserialize(YAML::Node const& node) -> void override;
};
}
