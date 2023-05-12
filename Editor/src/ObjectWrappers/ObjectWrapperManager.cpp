#include "ObjectWrapperManager.hpp"


namespace leopph::editor {
auto ObjectWrapperManager::Create() noexcept -> ObjectWrapperManager {
  ObjectWrapperManager factoryManager;
  factoryManager.Register<Entity>();
  factoryManager.Register<TransformComponent>();
  factoryManager.Register<CameraComponent>();
  factoryManager.Register<BehaviorComponent>();
  factoryManager.Register<StaticMeshComponent>();
  factoryManager.Register<LightComponent>();
  factoryManager.Register<Material>();
  factoryManager.Register<Mesh>();
  factoryManager.Register<Texture2D>();
  factoryManager.Register<Scene>();
  factoryManager.Register<Cubemap>();
  factoryManager.Register<SkyboxComponent>();
  return factoryManager;
}
}
