#include "ModelComponent.hpp"

#include "Systems.hpp"

namespace leopph {
Object::Type const ModelComponent::SerializationType{ Object::Type::CubeModel };

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
	return Type::CubeModel;
}


auto ModelComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "CubeModel");
}
}
