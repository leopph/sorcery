// ReSharper disable All
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
// ReSharper restore All

#include "PhysicsManager.hpp"

#include <PxPhysicsAPI.h>

#include <stdexcept>


namespace sorcery {
class PhysicsManager::Impl {
  physx::PxFoundation* mFoundation{ nullptr };
  physx::PxPhysics* mPhysics{ nullptr };
  physx::PxDefaultAllocator mDefaultAllocatorCallback;
  physx::PxDefaultErrorCallback mDefaultErrorCallback;

public:
  auto StartUp() -> void {
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mDefaultErrorCallback);

    if (!mFoundation) {
      throw std::runtime_error{ "Failed to create PhysX Foundation." };
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, nullptr);

    if (!mPhysics) {
      throw std::runtime_error{ "Failed to create PhysX object." };
    }
  }


  auto ShutDown() -> void {
    mPhysics->release();
    mPhysics = nullptr;

    mFoundation->release();
    mFoundation = nullptr;
  }
};


PhysicsManager::PhysicsManager() :
  mImpl{ new Impl{} } { }


PhysicsManager::~PhysicsManager() {
  delete mImpl;
}


auto PhysicsManager::StartUp() const -> void {
  mImpl->StartUp();
}


auto PhysicsManager::ShutDown() const -> void {
  mImpl->ShutDown();
}
}
