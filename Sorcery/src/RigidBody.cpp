#include "RigidBody.hpp"
#include "Systems.hpp"
#include "PhysicsManager.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::RigidBody>{ "RigidBody" }
    .constructor<>();
}


namespace sorcery {
Object::Type const RigidBody::SerializationType{ Object::Type::StaticRigidBody };


auto RigidBody::GetSerializationType() const -> Type {
  return SerializationType;
}


auto RigidBody::Serialize(YAML::Node& node) const -> void {
  Component::Serialize(node);
}


auto RigidBody::Deserialize(YAML::Node const& node) -> void {
  Component::Deserialize(node);
}


RigidBody::RigidBody() :
  mInternalPtr{ gPhysicsManager.CreateInternalRigidBody(this) } {}


RigidBody::~RigidBody() {
  gPhysicsManager.DestroyInternalRigidBody(mInternalPtr);
}
}
