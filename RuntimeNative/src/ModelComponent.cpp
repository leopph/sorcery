#include "ModelComponent.hpp"

#include "Systems.hpp"

namespace leopph {
Object::Type const ModelComponent::SerializationType{ Object::Type::Model };

ModelComponent::ModelComponent() :
	mMat{ gRenderer.GetDefaultMaterial().get() },
	mMesh{ gRenderer.GetCubeMesh().get() } {
	gRenderer.RegisterCubeModel(this);
}


ModelComponent::~ModelComponent() {
	gRenderer.UnregisterCubeModel(this);
}


auto ModelComponent::GetMaterial() const noexcept -> Material& {
	return *mMat;
}


auto ModelComponent::SetMaterial(Material& material) noexcept -> void {
	mMat = &material;
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
	node["material"] = mMat->GetGuid().ToString();
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

	if (node["material"]) {
		if (auto const guidStr{ node["material"].as<std::string>() }; !guidStr.empty()) {
			mMat = dynamic_cast<Material*>(FindObjectByGuid(Guid::Parse(guidStr)));

			if (!mMat) {
				mMat = gRenderer.GetDefaultMaterial().get();
			}
		}
	}
	else {
		mMat = gRenderer.GetDefaultMaterial().get();
	}
}
}
