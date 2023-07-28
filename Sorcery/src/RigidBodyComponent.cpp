#include "RigidBodyComponent.hpp"
#include "PhysicsManager.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::RigidBodyComponent>{ "Rigid Body Component" }
    .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}


namespace sorcery {
RigidBodyComponent::RigidBodyComponent() :
  mInternalPtr{ gPhysicsManager.CreateInternalRigidBody(this) } {}


RigidBodyComponent::~RigidBodyComponent() {
  gPhysicsManager.DestroyInternalRigidBody(mInternalPtr);
}
}
