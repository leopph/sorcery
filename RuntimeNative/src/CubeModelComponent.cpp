#include "CubeModelComponent.hpp"

#include "Systems.hpp"

namespace leopph {
Object::Type const CubeModelComponent::SerializationType{ Object::Type::CubeModel };

CubeModelComponent::CubeModelComponent() :
	mMat{ gRenderer.GetDefaultMaterial().get() },
	mMesh{ gRenderer.GetCubeMesh().get() } {
	gRenderer.RegisterCubeModel(this);
}


CubeModelComponent::~CubeModelComponent() {
	gRenderer.UnregisterCubeModel(this);
}


auto CubeModelComponent::GetMaterial() const noexcept -> Material& {
	return *mMat;
}


auto CubeModelComponent::SetMaterial(Material& material) noexcept -> void {
	mMat = &material;
}

auto CubeModelComponent::GetMesh() const noexcept -> Mesh& {
	return *mMesh;
}

auto CubeModelComponent::SetMesh(Mesh& mesh) noexcept -> void {
	mMesh = &mesh;
}

auto CubeModelComponent::GetSerializationType() const -> Type {
	return Type::CubeModel;
}


auto CubeModelComponent::CreateManagedObject() -> void {
	return ManagedAccessObject::CreateManagedObject("leopph", "CubeModel");
}
}
