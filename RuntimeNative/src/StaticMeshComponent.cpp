#include "StaticMeshComponent.hpp"

#include "Entity.hpp"
#include "Renderer.hpp"
#include "TransformComponent.hpp"

#include <format>
#include <stdexcept>


namespace leopph {
Object::Type const StaticMeshComponent::SerializationType{ Type::StaticMesh };


auto StaticMeshComponent::AdjustMaterialListForMesh() -> void {
	if (std::size_t const subMeshCount{ std::size(mMesh->GetSubMeshes()) }, mtlCount{ std::size(mMesh->GetSubMeshes()) }; subMeshCount != mtlCount) {
		mMaterials.resize(subMeshCount);

		for (std::size_t i{ mtlCount }; i < subMeshCount; i++) {
			mMaterials[i] = renderer::GetDefaultMaterial();
		}
	}
}


StaticMeshComponent::StaticMeshComponent() :
	mMesh{ renderer::GetCubeMesh() } {
	AdjustMaterialListForMesh();
	renderer::RegisterStaticMesh(this);
}


StaticMeshComponent::~StaticMeshComponent() {
	renderer::UnregisterStaticMesh(this);
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::span<Material* const> {
	return mMaterials;
}


auto StaticMeshComponent::SetMaterials(std::vector<Material*> materials) -> void {
	for (Material const* const mtl : materials) {
		if (!mtl) {
			throw std::runtime_error{ "Found nullptr while attempting to materials on StaticMeshComponent." };
		}
	}

	mMaterials = std::move(materials);
	AdjustMaterialListForMesh();
}


auto StaticMeshComponent::ReplaceMaterial(int const idx, Material& mtl) -> void {
	if (idx >= std::ssize(mMaterials)) {
		throw std::runtime_error{ std::format("Invalid index {} while attempting to replace material on StaticMeshComponent.", idx) };
	}

	mMaterials[idx] = &mtl;
}


auto StaticMeshComponent::GetMesh() const noexcept -> Mesh& {
	return *mMesh;
}


auto StaticMeshComponent::SetMesh(Mesh& mesh) noexcept -> void {
	mMesh = &mesh;
	AdjustMaterialListForMesh();
}


auto StaticMeshComponent::GetSerializationType() const -> Type {
	return Type::StaticMesh;
}


auto StaticMeshComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "StaticMesh");
}


auto StaticMeshComponent::Serialize(YAML::Node& node) const -> void {
	Component::Serialize(node);
	node["mesh"] = mMesh->GetGuid().ToString();

	for (auto const mtl : mMaterials) {
		node["materials"].push_back(mtl->GetGuid().ToString());
	}
}


auto StaticMeshComponent::Deserialize(YAML::Node const& node) -> void {
	Component::Deserialize(node);

	if (node["mesh"]) {
		if (auto const guidStr{ node["mesh"].as<std::string>() }; !guidStr.empty()) {
			mMesh = dynamic_cast<Mesh*>(FindObjectByGuid(Guid::Parse(guidStr)));

			if (!mMesh) {
				mMesh = renderer::GetCubeMesh();
			}
		}
	}
	else {
		mMesh = renderer::GetCubeMesh();
	}

	if (auto const mtlListNode{ node["materials"] }) {
		for (auto const mtlNode : mtlListNode) {
			if (auto const guidStr{ mtlNode.as<std::string>() }; !guidStr.empty()) {
				if (auto const mtl{ dynamic_cast<Material*>(FindObjectByGuid(Guid::Parse(guidStr))) }) {
					mMaterials.emplace_back(mtl);
				}
			}
		}
	}

	AdjustMaterialListForMesh();
}


auto StaticMeshComponent::CalculateBounds() const noexcept -> AABB {
	auto const& localBounds{ mMesh->GetBounds() };
	auto const modelMtx{ GetEntity()->GetTransform().GetModelMatrix() };
	auto boundsVertices{ localBounds.CalculateVertices() };

	for (auto& vertex : boundsVertices) {
		vertex = Vector3{ Vector4{ vertex, 1 } * modelMtx };
	}

	return AABB::FromVertices(boundsVertices);
}
}
