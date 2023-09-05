#include "RigidBodyComponent.hpp"
#include "../PhysicsManager.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::RigidBodyComponent>{"Rigid Body Component"}
    .constructor<>()(rttr::policy::ctor::as_raw_ptr);
}


namespace sorcery {
auto RigidBodyComponent::OnInit() -> void {
  Component::OnInit();
  mInternalPtr = gPhysicsManager.CreateInternalRigidBody(this);
}


auto RigidBodyComponent::OnDestroy() -> void {
  gPhysicsManager.DestroyInternalRigidBody(mInternalPtr);
  Component::OnDestroy();
}
}
