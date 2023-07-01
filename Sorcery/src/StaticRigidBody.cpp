#include "StaticRigidBody.hpp"
#include "Systems.hpp"
#include "PhysicsManager.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::StaticRigidBody>{ "StaticRigidBody" }
    .constructor<>();
}


namespace sorcery {
Object::Type const StaticRigidBody::SerializationType{ Object::Type::StaticRigidBody };


auto StaticRigidBody::GetSerializationType() const -> Type {
  return SerializationType;
}


auto StaticRigidBody::Serialize(YAML::Node& node) const -> void {
  Component::Serialize(node);
}


auto StaticRigidBody::Deserialize(YAML::Node const& node) -> void {
  Component::Deserialize(node);
}


StaticRigidBody::StaticRigidBody() :
  mInternalPtr{ gPhysicsManager.CreateInternalStaticRigidBody() } {}


StaticRigidBody::~StaticRigidBody() {
  gPhysicsManager.DestroyInternalStaticRigidBody(mInternalPtr);
}
}
