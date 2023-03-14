#include "ModelComponent.hpp"

#include "Systems.hpp"

namespace leopph {
Object::Type const ModelComponent::SerializationType{ Object::Type::Model };

ModelComponent::ModelComponent() :
	mMesh{ gRenderer.GetCubeMesh().get() } {
	AddMaterial(*gRenderer.GetDefaultMaterial());
	gRenderer.RegisterCubeModel(this);
}


ModelComponent::~ModelComponent() {
	gRenderer.UnregisterCubeModel(this);
}


auto ModelComponent::GetMaterials() const noexcept -> std::span<Material* const> {
	return mMaterials;
}

auto ModelComponent::AddMaterial(Material& mtl) noexcept -> void {
	mMaterials.push_back(&mtl);
}

auto ModelComponent::RemoveMaterial(int const idx) noexcept -> void {
	if (idx < mMaterials.size()) {
		mMaterials.erase(std::begin(mMaterials) + idx);
	}
}

auto ModelComponent::ReplaceMaterial(int const idx, Material& mtl) noexcept -> void {
	if (idx < mMaterials.size()) {
		mMaterials[idx] = &mtl;
	}
}

auto ModelComponent::SetMaterials(std::vector<Material*> materials) noexcept -> void {
	for (auto const mtl : materials) {
		if (!mtl) {
			return;
		}
	}

	mMaterials = std::move(materials);
}

auto ModelComponent::GetMesh() const noexcept -> Mesh& {
	return *mMesh;
}

auto ModelComponent::SetMesh(Mesh& mesh) noexcept -> void {
	mMesh = &mesh;
}

auto ModelComponent::GetSerializationType() const -> Type {
	return Type::Model;
}


auto ModelComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "Model");
}

auto ModelComponent::Serialize(YAML::Node& node) const -> void {
	Component::Serialize(node);
	node["mesh"] = mMesh->GetGuid().ToString();

	for (auto const mtl : mMaterials) {
		node["materials"].push_back(mtl->GetGuid().ToString());
	}
}

auto ModelComponent::Deserialize(YAML::Node const& node) -> void {
	Component::Deserialize(node);

	if (node["mesh"]) {
		if (auto const guidStr{ node["mesh"].as<std::string>() }; !guidStr.empty()) {
			mMesh = dynamic_cast<Mesh*>(FindObjectByGuid(Guid::Parse(guidStr)));

			if (!mMesh) {
				mMesh = gRenderer.GetCubeMesh().get();
			}
		}
	}
	else {
		mMesh = gRenderer.GetCubeMesh().get();
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
		AddMaterial(*gRenderer.GetDefaultMaterial());
	}
}
}
