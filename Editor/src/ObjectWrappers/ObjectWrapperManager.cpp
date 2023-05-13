#include "ObjectWrapperManager.hpp"

#include <ranges>


namespace leopph::editor {
auto ObjectWrapperManager::Create() noexcept -> std::shared_ptr<ObjectWrapperManager> {
  auto ret{ std::shared_ptr<ObjectWrapperManager>{ new ObjectWrapperManager{} } };

  ret->Register<Entity>();
  ret->Register<TransformComponent>();
  ret->Register<CameraComponent>();
  ret->Register<BehaviorComponent>();
  ret->Register<StaticMeshComponent>();
  ret->Register<LightComponent>();
  ret->Register<Material>();
  ret->Register<Mesh>();
  ret->Register<Texture2D>();
  ret->Register<Scene>();
  ret->Register<Cubemap>();
  ret->Register<SkyboxComponent>();

  return ret;
}


auto ObjectWrapperManager::GetWrappers(std::vector<ObserverPtr<ObjectWrapper>>& outWrappers) const -> std::vector<ObserverPtr<ObjectWrapper>>& {
  for (auto const& wrapper : mTypeIdxToWrapper | std::views::values) {
    outWrappers.emplace_back(wrapper.get());
  }

  return outWrappers;
}
}
