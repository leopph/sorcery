#include "Scene.hpp"

#include "../engine_context.hpp"
#include "../scene_objects/SceneObject.hpp"
#include "../Platform.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../job_system.hpp"
#include "../ResourceManager.hpp"

#include <algorithm>
#include <ranges>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"}.property("skybox", &sorcery::Scene::GetSkybox,
    &sorcery::Scene::SetSkybox);
}


namespace sorcery {
Scene* Scene::sActiveScene{nullptr};
std::vector<Scene*> Scene::sAllScenes;


auto Scene::GetActiveScene() noexcept -> Scene* {
  if (!sActiveScene) {
    sActiveScene = CreateInit<Scene>().release();
  }

  return sActiveScene;
}


Scene::Scene() {
  sAllScenes.emplace_back(this);

  if (!sActiveScene) {
    sActiveScene = this;
  }

  Save();
}


Scene::~Scene() {
  std::erase(sAllScenes, this);

  if (sActiveScene == this) {
    sActiveScene = sAllScenes.empty() ? nullptr : sAllScenes.back();
  }

  Clear();
}


auto Scene::Serialize() const noexcept -> YAML::Node {
  return mYamlData;
}


auto Scene::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  mYamlData = Clone(yamlNode);
}


auto Scene::AddEntity(Entity& entity) -> void {
  mEntities.push_back(std::addressof(entity));
}


auto Scene::RemoveEntity(Entity const& entity) -> void {
  std::erase(mEntities, std::addressof(entity));
}


auto Scene::GetEntities() const noexcept -> std::vector<Entity*> const& {
  return mEntities;
}


auto Scene::Save() -> void {
  static std::vector<SceneObject*> tmpThisSceneObjects;
  tmpThisSceneObjects.clear();

  static std::vector<Component*> tmpComponents;
  tmpComponents.clear();

  static std::unordered_map<void const*, int> ptrFixUp;
  ptrFixUp.clear();

  mYamlData.reset();
  mYamlData["version"] = 1;
  mYamlData["ambientLight"] = mAmbientLight;
  mYamlData["skyMode"] = static_cast<int>(mSkyMode);
  mYamlData["skyColor"] = mSkyColor;
  mYamlData["skybox"] = mSkybox ? mSkybox->GetGuid() : Guid::Invalid();

  for (auto const entity : mEntities) {
    tmpThisSceneObjects.emplace_back(entity);

    for (auto const component : entity->GetComponents(tmpComponents)) {
      tmpThisSceneObjects.emplace_back(component);
    }
  }

  for (auto const sceneObj : tmpThisSceneObjects) {
    ptrFixUp[sceneObj] = static_cast<int>(std::ssize(ptrFixUp) + 1);
  }

  auto const extensionFunc{
    [](rttr::variant const& v) -> YAML::Node {
      YAML::Node retNode;

      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        auto const it{ptrFixUp.find(v.get_value<SceneObject*>())};
        retNode = it != std::end(ptrFixUp) ? it->second : 0;
      }

      return retNode;
    }
  };

  for (auto const sceneObj : tmpThisSceneObjects) {
    YAML::Node sceneObjNode;
    sceneObjNode["type"] = rttr::type::get(*sceneObj).get_name().to_string();
    sceneObjNode["properties"] = ReflectionSerializeToYaml(*sceneObj, extensionFunc);
    mYamlData["sceneObjects"].push_back(sceneObjNode);
  }
}


auto Scene::Load() -> void {
  static std::unordered_map<int, SceneObject*> ptrFixUp;
  ptrFixUp.clear();

  auto const lastActiveScene{sActiveScene};
  sActiveScene = this;

  Clear();

  if (auto const version{mYamlData["version"]}; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{
      std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName())
    };
  }

  if (auto const node{mYamlData["ambientLight"]}) {
    mAmbientLight = node.as<Vector3>(mAmbientLight);
  }

  if (auto const node{mYamlData["skyMode"]}) {
    mSkyMode = static_cast<SkyMode>(node.as<int>(static_cast<int>(mSkyMode)));
  }

  if (auto const node{mYamlData["skyColor"]}) {
    mSkyColor = node.as<Vector3>(mSkyColor);
  }

  struct SkyboxJobData {
    Guid guid;
    Cubemap* cubemap;
  } skybox_job_data;

  Job* skybox_job{nullptr};

  if (auto const node{mYamlData["skybox"]}) {
    if (auto const guid{node.as<Guid>(Guid::Invalid())}; guid.IsValid()) {
      skybox_job_data.guid = guid;

      skybox_job = g_engine_context.job_system->CreateJob([](void const* const data_ptr) {
        auto& job_data{**static_cast<SkyboxJobData* const*>(data_ptr)};
        job_data.cubemap = g_engine_context.resource_manager->GetOrLoad<Cubemap>(job_data.guid);
      }, &skybox_job_data);

      g_engine_context.job_system->Run(skybox_job);
    }
  }

  std::function<void(YAML::Node const&, rttr::type const&, std::vector<Guid>&)> discover_resource_references;
  discover_resource_references =
    [&discover_resource_references](YAML::Node const& node, rttr::type const& type, std::vector<Guid>& guids) -> void {
      if (!node.IsDefined()) {
        return;
      }

      auto const actual_type{type.is_wrapper() ? type.get_wrapped_type() : type};

      if (node.IsScalar() && actual_type.is_pointer() && actual_type.get_raw_type().is_derived_from(
            rttr::type::get<Resource>())) {
        if (auto const guid{node.as<Guid>(Guid::Invalid())}; guid.IsValid()) {
          guids.emplace_back(guid);
          return;
        }
      }

      if (node.IsSequence() && actual_type.is_sequential_container()) {
        if (auto const template_args{actual_type.get_template_arguments()}; !template_args.empty()) {
          for (auto const& elem : node) {
            discover_resource_references(elem, *template_args.begin(), guids);
          }
        }

        return;
      }

      if (node.IsMap() && actual_type.is_class()) {
        for (auto const& prop : actual_type.get_properties()) {
          discover_resource_references(node[prop.get_name().to_string()], prop.get_type(), guids);
        }
      }
    };

  std::vector<Guid> resource_guids;

  for (auto const& sceneObjectNode : mYamlData["sceneObjects"]) {
    auto const typeNode{sceneObjectNode["type"]};
    auto const type{rttr::type::get_by_name(typeNode.as<std::string>())};
    auto const sceneObjectVariant{type.create()};
    auto const sceneObj{sceneObjectVariant.get_value<SceneObject*>()};
    ptrFixUp[static_cast<int>(std::ssize(ptrFixUp)) + 1] = sceneObj;
    discover_resource_references(sceneObjectNode["properties"], type, resource_guids);
  }

  std::ranges::sort(resource_guids, [](Guid const& lhs, Guid const& rhs) {
    return lhs < rhs;
  });

  resource_guids.erase(std::ranges::unique(resource_guids, [](Guid const& lhs, Guid const& rhs) {
    return lhs <=> rhs == std::strong_ordering::equal;
  }).begin(), resource_guids.end());

  struct ResourceJobData {
    Guid guid;
    Resource* resource;
  };

  std::vector<ResourceJobData> resource_job_data;
  resource_job_data.reserve(resource_guids.size());

  std::vector<Job*> resource_jobs;
  resource_jobs.reserve(resource_guids.size());

  for (auto const& guid : resource_guids) {
    resource_job_data.emplace_back(guid);

    resource_jobs.emplace_back(g_engine_context.job_system->CreateJob([](void const* const data_ptr) {
      auto& job_data{**static_cast<ResourceJobData* const*>(data_ptr)};
      job_data.resource = g_engine_context.resource_manager->GetOrLoad<Resource>(job_data.guid);
    }, &resource_job_data.back()));

    g_engine_context.job_system->Run(resource_jobs.back());
  }

  auto const extensionFunc{
    [](YAML::Node const& objNode, rttr::variant& v) -> void {
      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        if (auto const it{ptrFixUp.find(objNode.as<int>(0))}; it != std::end(ptrFixUp)) {
          auto const type{v.get_type()};
          v = it->second;
          [[maybe_unused]] auto const success{v.convert(type)};
          assert(success);
        }
      }
    }
  };

  struct SceneObjectJobData {
    YAML::Node prop_node;
    Object* obj;
    decltype(extensionFunc) ext_func;
  };

  std::vector<SceneObjectJobData> job_data;
  job_data.reserve(ptrFixUp.size());

  std::vector<Job*> loader_jobs;
  loader_jobs.reserve(ptrFixUp.size());

  for (auto const& [fileId, obj] : ptrFixUp) {
    job_data.emplace_back(mYamlData["sceneObjects"][fileId - 1]["properties"], obj, extensionFunc);

    loader_jobs.emplace_back(g_engine_context.job_system->CreateJob([](void const* const data_ptr) {
      auto const& job_data{**static_cast<SceneObjectJobData const* const*>(data_ptr)};
      ReflectionDeserializeFromYaml(job_data.prop_node, *job_data.obj, job_data.ext_func);
    }, &job_data.back()));

    g_engine_context.job_system->Run(loader_jobs.back());
  }

  for (auto const* const job : loader_jobs) {
    g_engine_context.job_system->Wait(job);
  }

  for (auto* const obj : ptrFixUp | std::views::values) {
    obj->Initialize();
  }

  if (skybox_job) {
    g_engine_context.job_system->Wait(skybox_job);
    mSkybox = skybox_job_data.cubemap;
  }

  sActiveScene = lastActiveScene;
}


auto Scene::SetActive() -> void {
  sActiveScene = this;
}


auto Scene::Clear() -> void {
  // Entity destructor modifies this collection, hence the strange loop
  while (!mEntities.empty()) {
    delete mEntities.back();
  }
}


auto Scene::GetAmbientLightVector() const noexcept -> Vector3 const& {
  return mAmbientLight;
}


auto Scene::SetAmbientLightVector(Vector3 const& vector) noexcept -> void {
  mAmbientLight = vector;
}


auto Scene::GetAmbientLight() const noexcept -> Color {
  return Color{Vector4{mAmbientLight, 1}};
}


auto Scene::SetAmbientLight(Color const& color) noexcept -> void {
  mAmbientLight = Vector3{static_cast<Vector4>(color)};
}


auto Scene::GetSkyMode() const noexcept -> SkyMode {
  return mSkyMode;
}


auto Scene::SetSkyMode(SkyMode const skyMode) noexcept -> void {
  mSkyMode = skyMode;
}


auto Scene::GetSkyColor() const noexcept -> Vector3 const& {
  return mSkyColor;
}


auto Scene::SetSkyColor(Vector3 const& skyColor) noexcept -> void {
  mSkyColor = skyColor;
}


auto Scene::GetSkybox() const noexcept -> Cubemap* {
  return mSkybox;
}


auto Scene::SetSkybox(Cubemap* const skybox) noexcept -> void {
  mSkybox = skybox;
}
}
