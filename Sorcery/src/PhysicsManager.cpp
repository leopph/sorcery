// ReSharper disable All
#define _SILENCE_CXX20_CISO646_REMOVED_WARNING
// ReSharper restore All

#include "PhysicsManager.hpp"

#include <format>
#include <memory>
#include <stdexcept>
#include <vector>

#include "physx/PxPhysicsAPI.h"

#include "Entity.hpp"
#include "Timing.hpp"
#include "TransformComponent.hpp"


namespace sorcery {
PhysicsManager gPhysicsManager;


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
  std::vector<std::unique_ptr<InternalRigidBody>> mInternalRigidBodies;


  [[nodiscard]] static auto ConvertTransformToPxTransform(TransformComponent const& transform) noexcept -> physx::PxTransform {
    physx::PxTransform pxTransform;
    pxTransform.p = physx::PxVec3{ transform.GetWorldPosition()[0], transform.GetWorldPosition()[1], transform.GetWorldPosition()[2] };
    pxTransform.q = physx::PxQuat{ transform.GetWorldRotation().x, transform.GetWorldRotation().y, transform.GetWorldRotation().z, transform.GetWorldRotation().w };
    return pxTransform;
  }


  static auto ConvertPxTransformToTransform(physx::PxTransform const& pxTransform, TransformComponent& transform) noexcept -> void {
    transform.SetWorldPosition(Vector3{ pxTransform.p.x, pxTransform.p.y, pxTransform.p.z });
    transform.SetWorldRotation(Quaternion{ pxTransform.q.w, pxTransform.q.x, pxTransform.q.y, pxTransform.q.z });
  }

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
    for (auto const& rigidBody : mInternalRigidBodies) {
      auto const pxRigidDynamic{ static_cast<physx::PxRigidDynamic*>(rigidBody->mData) };
      auto const owningComponent{ static_cast<Component*>(pxRigidDynamic->userData) };
      auto const& transform{ owningComponent->GetEntity().GetTransform() };
      pxRigidDynamic->setGlobalPose(ConvertTransformToPxTransform(transform));
    }

    auto const frameTime{ timing::GetFrameTime() };
    mAccumTime += frameTime;

    while (mAccumTime >= mSimStepSize) {
      mAccumTime -= mSimStepSize;
      mScene->simulate(mSimStepSize);
      mScene->fetchResults(true);
    }

    for (auto const& rigidBody : mInternalRigidBodies) {
      auto const pxRigidDynamic{ static_cast<physx::PxRigidDynamic*>(rigidBody->mData) };
      auto const owningComponent{ static_cast<Component*>(pxRigidDynamic->userData) };
      auto& transform{ owningComponent->GetEntity().GetTransform() };
      ConvertPxTransformToTransform(pxRigidDynamic->getGlobalPose(), transform);
    }
  }


  [[nodiscard]] auto CreateInternalRigidBody(ObserverPtr<Component> const owningComponent) -> ObserverPtr<InternalRigidBody> {
    auto const pxRigidDynamic{ mPhysics->createRigidDynamic(physx::PxTransform{ physx::PxIdentity }) };
    pxRigidDynamic->userData = owningComponent;
    mScene->addActor(*pxRigidDynamic);

    auto const& ret{ mInternalRigidBodies.emplace_back(std::make_unique<InternalRigidBody>()) };
    ret->mData = pxRigidDynamic;
    return ret.get();
  }


  auto DestroyInternalRigidBody(ObserverPtr<InternalRigidBody> const internalRigidBody) -> void {
    auto const pxRigidDynamic{ static_cast<physx::PxRigidDynamic*>(internalRigidBody->mData) };
    mScene->removeActor(*pxRigidDynamic);
    pxRigidDynamic->release();

    std::erase_if(mInternalRigidBodies, [internalRigidBody](auto const& elem) {
      return elem.get() == internalRigidBody;
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


auto PhysicsManager::CreateInternalRigidBody(ObserverPtr<Component> const owningComponent) const -> ObserverPtr<InternalRigidBody> {
  return mImpl->CreateInternalRigidBody(owningComponent);
}


auto PhysicsManager::DestroyInternalRigidBody(ObserverPtr<InternalRigidBody> const internalRigidBody) const -> void {
  return mImpl->DestroyInternalRigidBody(internalRigidBody);
}
}
