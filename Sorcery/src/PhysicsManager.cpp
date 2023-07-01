// ReSharper disable All
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
// ReSharper restore All

#include "PhysicsManager.hpp"

#include <format>
#include <memory>
#include <stdexcept>
#include <vector>

#include <PxPhysicsAPI.h>

#include "Timing.hpp"


namespace sorcery {
namespace {
class PhysXErrorCallback : public physx::PxErrorCallback {
  auto reportError([[maybe_unused]] physx::PxErrorCode::Enum code, char const* message, [[maybe_unused]] char const* file, [[maybe_unused]] int line) -> void override {
    throw std::runtime_error{ std::format("PhysX error: {}", message) };
  }
};
}


class PhysicsManager::Impl {
  physx::PxFoundation* mFoundation{ nullptr };
  physx::PxPhysics* mPhysics{ nullptr };
  physx::PxDefaultAllocator mDefaultAllocatorCallback;
  PhysXErrorCallback mErrorCallback;
  physx::PxScene* mScene{ nullptr };
  float mAccumTime{ 0 };
  float mSimStepSize{ 1.0f / 60.0f };
  std::vector<std::unique_ptr<InternalStaticRigidBody>> mStaticRigidBodies;

public:
  auto StartUp() -> void {
    mFoundation = PxCreateFoundation(PX_PHYSICS_VERSION, mDefaultAllocatorCallback, mErrorCallback);

    if (!mFoundation) {
      throw std::runtime_error{ "Failed to create PhysX Foundation." };
    }

    mPhysics = PxCreatePhysics(PX_PHYSICS_VERSION, *mFoundation, physx::PxTolerancesScale(), true, nullptr);

    if (!mPhysics) {
      throw std::runtime_error{ "Failed to create PhysX object." };
    }

    physx::PxSceneDesc sceneDesc{ mPhysics->getTolerancesScale() };
    sceneDesc.filterShader = physx::PxDefaultSimulationFilterShader;
    sceneDesc.cpuDispatcher = physx::PxDefaultCpuDispatcherCreate(1);

    mScene = mPhysics->createScene(sceneDesc);

    if (!mScene) {
      throw std::runtime_error{ "Failed to create PhysX scene." };
    }
  }


  auto ShutDown() -> void {
    mPhysics->release();
    mPhysics = nullptr;

    mFoundation->release();
    mFoundation = nullptr;
  }


  auto Update() -> void {
    auto const frameTime{ timing::GetFrameTime() };
    mAccumTime += frameTime;

    while (mAccumTime >= mSimStepSize) {
      mAccumTime -= mSimStepSize;
      mScene->simulate(mSimStepSize);
      mScene->fetchResults(true);
    }
  }


  [[nodiscard]] auto CreateInternalStaticRigidBody() -> ObserverPtr<InternalStaticRigidBody> {
    auto const pxRigidStatic{ mPhysics->createRigidStatic(physx::PxTransform{ physx::PxIdentity }) };
    mScene->addActor(*pxRigidStatic);

    auto const& ret{ mStaticRigidBodies.emplace_back(std::make_unique<InternalStaticRigidBody>()) };
    ret->mData = pxRigidStatic;
    return ret.get();
  }


  auto DestroyInternalStaticRigidBody(ObserverPtr<InternalStaticRigidBody> const internalStaticRigidBody) -> void {
    auto const pxRigidStatic{ static_cast<physx::PxRigidStatic*>(internalStaticRigidBody->mData) };
    mScene->removeActor(*pxRigidStatic);
    pxRigidStatic->release();

    std::erase_if(mStaticRigidBodies, [internalStaticRigidBody](auto const& elem) {
      return elem.get() == internalStaticRigidBody;
    });
  }
};


PhysicsManager::PhysicsManager() :
  mImpl{ new Impl{} } {}


PhysicsManager::~PhysicsManager() {
  delete mImpl;
}


auto PhysicsManager::StartUp() const -> void {
  mImpl->StartUp();
}


auto PhysicsManager::ShutDown() const -> void {
  mImpl->ShutDown();
}


auto PhysicsManager::Update() const -> void {
  mImpl->Update();
}


auto PhysicsManager::CreateInternalStaticRigidBody() const -> ObserverPtr<InternalStaticRigidBody> {
  return mImpl->CreateInternalStaticRigidBody();
}


auto PhysicsManager::DestroyInternalStaticRigidBody(ObserverPtr<InternalStaticRigidBody> const internalStaticRigidBody) const -> void {
  return mImpl->DestroyInternalStaticRigidBody(internalStaticRigidBody);
}
}
