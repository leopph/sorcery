#include "StaticMeshComponent.hpp"

#include "Renderer.hpp"

namespace leopph {
Object::Type const StaticMeshComponent::SerializationType{ Type::StaticMesh };


StaticMeshComponent::StaticMeshComponent() :
	mMesh{ renderer::GetCubeMesh() } {
	AddMaterial(*renderer::GetDefaultMaterial());
	renderer::RegisterStaticMesh(this);
}


StaticMeshComponent::~StaticMeshComponent() {
	renderer::UnregisterStaticMesh(this);
}


auto StaticMeshComponent::GetMaterials() const noexcept -> std::span<Material* const> {
	return mMaterials;
}


auto StaticMeshComponent::AddMaterial(Material& mtl) noexcept -> void {
	mMaterials.push_back(&mtl);
}


auto StaticMeshComponent::RemoveMaterial(int const idx) noexcept -> void {
	if (idx < mMaterials.size()) {
		mMaterials.erase(std::begin(mMaterials) + idx);
	}
}


auto StaticMeshComponent::ReplaceMaterial(int const idx, Material& mtl) noexcept -> void {
	if (idx < mMaterials.size()) {
		mMaterials[idx] = &mtl;
	}
}


auto StaticMeshComponent::SetMaterials(std::vector<Material*> materials) noexcept -> void {
	for (auto const mtl : materials) {
		if (!mtl) {
			return;
		}
	}

	mMaterials = std::move(materials);
}


auto StaticMeshComponent::GetMesh() const noexcept -> Mesh& {
	return *mMesh;
}


auto StaticMeshComponent::SetMesh(Mesh& mesh) noexcept -> void {
	mMesh = &mesh;
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

	mMaterials.clear();

	if (auto const mtlListNode{ node["materials"] }) {
		for (auto const mtlNode : mtlListNode) {
			if (auto const guidStr{ mtlNode.as<std::string>() }; !guidStr.empty()) {
				if (auto const mtl{ dynamic_cast<Material*>(FindObjectByGuid(Guid::Parse(guidStr))) }) {
					AddMaterial(*mtl);
				}
			}
		}
	}

	if (mMaterials.empty()) {
		AddMaterial(*renderer::GetDefaultMaterial());
	}
}
}
