#include "Scene.hpp"

#include "../app.hpp"
#include "../Platform.hpp"
#include "../Serialization.hpp"
#include "../scene_objects/SceneObject.hpp"
#undef FindResource
#include "../job_system.hpp"
#include "../Reflection.hpp"
#include "../ResourceManager.hpp"

#include <algorithm>
#include <ranges>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"}
    .property("skybox", &sorcery::Scene::GetSkybox, &sorcery::Scene::SetSkybox);
}


namespace sorcery {
Scene* Scene::active_scene_{nullptr};
std::vector<Scene*> Scene::all_scenes_;


auto detail::GetIrradianceMap(Scene const& scene) -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return scene.irradiance_map_;
}


auto detail::RecreateIrradianceMap(Scene& scene, graphics::GraphicsDevice& device, DXGI_FORMAT const format) -> void {
  if (!scene.skybox_) {
    return;
  }

  auto const& skybox_desc{scene.skybox_->GetTex()->GetDesc()};

  scene.irradiance_map_ = device.CreateTexture(graphics::TextureDesc{
      .dimension = graphics::TextureDimension::kCube,
      .width = skybox_desc.width,
      .height = skybox_desc.height,
      .depth_or_array_size = 6,
      .mip_levels = 1,
      .format = format,
      .sample_count = 1,
      .depth_stencil = false,
      .render_target = true,
      .shader_resource = true,
      .unordered_access = false
    }, graphics::CpuAccess::kNone,
    std::array{D3D12_CLEAR_VALUE{.Format = format, .Color = {0.0F, 0.0F, 0.0F, 1.0F}}}.data());
}


auto Scene::GetActiveScene() noexcept -> Scene* {
  return active_scene_;
}


Scene::Scene() {
  all_scenes_.emplace_back(this);

  if (!active_scene_) {
    active_scene_ = this;
  }

  Save();
}


Scene::~Scene() {
  for (auto const& entity : entities_) {
    entity->OnBeforeExitingScene(*this);
  }

  if (active_scene_ == this) {
    active_scene_ = all_scenes_.empty() ? nullptr : all_scenes_.back();
  }

  std::erase(all_scenes_, this);
}


auto Scene::Serialize() const noexcept -> YAML::Node {
  return yaml_data_;
}


auto Scene::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  yaml_data_ = Clone(yamlNode);
}


auto Scene::AddEntity(std::unique_ptr<Entity> entity) -> void {
  if (entity) {
    entities_.emplace_back(std::move(entity));
    entities_.back()->OnAfterEnteringScene(*this);
  }
}


auto Scene::RemoveEntity(Entity const& entity) -> std::unique_ptr<Entity> {
  if (auto const it{
    std::ranges::find_if(entities_, [&entity](std::unique_ptr<Entity> const& owned_entity) {
      return owned_entity.get() == std::addressof(entity);
    })
  }; it != std::end(entities_)) {
    (*it)->OnBeforeExitingScene(*this);
    auto ret{std::move(*it)};
    entities_.erase(it);
    return ret;
  }

  return nullptr;
}


auto Scene::GetEntities() const noexcept -> std::span<std::unique_ptr<Entity> const> {
  return entities_;
}


auto Scene::Save() -> void {
  static std::vector<SceneObject*> tmpThisSceneObjects;
  tmpThisSceneObjects.clear();

  static std::vector<Component*> tmpComponents;
  tmpComponents.clear();

  static std::unordered_map<void const*, int> ptrFixUp;
  ptrFixUp.clear();

  yaml_data_.reset();
  yaml_data_["version"] = 1;
  yaml_data_["ambientLight"] = ambient_light_;
  yaml_data_["skyMode"] = static_cast<int>(sky_mode_);
  yaml_data_["skyColor"] = sky_color_;
  yaml_data_["skybox"] = skybox_ ? skybox_->GetGuid() : Guid::Invalid();

  for (auto const& entity : entities_) {
    tmpThisSceneObjects.emplace_back(entity.get());

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
    yaml_data_["sceneObjects"].push_back(sceneObjNode);
  }
}


auto Scene::Load() -> void {
  if (auto const version{yaml_data_["version"]}; !version || !version.IsScalar() || version.as<int>(1) != 1) {
    throw std::runtime_error{
      std::format("Couldn't load scene \"{}\" because its version number is unsupported.", GetName())
    };
  }

  Clear();

  // Load scene settings

  if (auto const node{yaml_data_["ambientLight"]}) {
    ambient_light_ = node.as<Vector3>(ambient_light_);
  }

  if (auto const node{yaml_data_["skyMode"]}) {
    sky_mode_ = static_cast<SkyMode>(node.as<int>(static_cast<int>(sky_mode_)));
  }

  if (auto const node{yaml_data_["skyColor"]}) {
    sky_color_ = node.as<Vector3>(sky_color_);
  }

  // Start a job to load the skybox

  struct SkyboxJobData {
    Guid guid;
    Cubemap* cubemap;
  } skybox_job_data;

  ObserverPtr<Job> skybox_job;

  if (auto const node{yaml_data_["skybox"]}) {
    if (auto const guid{node.as<Guid>(Guid::Invalid())}; guid.IsValid()) {
      skybox_job_data.guid = guid;

      skybox_job = App::Instance().GetJobSystem().CreateJob([](SkyboxJobData* const data) {
        data->cubemap = App::Instance().GetResourceManager().GetOrLoad<Cubemap>(data->guid);
      }, &skybox_job_data);

      App::Instance().GetJobSystem().Run(skybox_job);
    }
  }

  // Discover all resource references in the scene objects and preload them

  std::vector<Guid> required_resource_guids;

  std::function<void(YAML::Node const&, rttr::type const&)> discover_resource_references;
  discover_resource_references = [&discover_resource_references, &required_resource_guids](
    YAML::Node const& node, rttr::type const& type) -> void {
      if (!node.IsDefined()) {
        return;
      }

      auto const actual_type{type.is_wrapper() ? type.get_wrapped_type() : type};

      if (node.IsScalar() && actual_type.is_pointer() && actual_type.get_raw_type().is_derived_from(
            rttr::type::get<Resource>())) {
        if (auto const guid{node.as<Guid>(Guid::Invalid())}; guid.IsValid()) {
          required_resource_guids.emplace_back(guid);
          return;
        }
      }

      if (node.IsSequence() && actual_type.is_sequential_container()) {
        if (auto const template_args{actual_type.get_template_arguments()}; !template_args.empty()) {
          for (auto const& elem : node) {
            discover_resource_references(elem, *template_args.begin());
          }
        }

        return;
      }

      if (node.IsMap() && actual_type.is_class()) {
        for (auto const& prop : actual_type.get_properties()) {
          discover_resource_references(node[prop.get_name().to_string()], prop.get_type());
        }
      }
    };


  // These pointers OWN the objects. A scene consists of entities and components.
  // Components will be taken ownership of by the entities during deserialization.
  // The scene will take ownership of the entities at the end of the process.
  std::vector<SceneObject*> scene_objects;

  static std::unordered_map<int, SceneObject*> ptr_fix_up;
  ptr_fix_up.clear();

  for (auto const& scene_obj_node : yaml_data_["sceneObjects"]) {
    auto const type_node{scene_obj_node["type"]};
    auto const type{rttr::type::get_by_name(type_node.as<std::string>())};
    auto const scene_obj{static_cast<SceneObject*>(Create(type).release())};
    scene_objects.emplace_back(scene_obj);
    ptr_fix_up[static_cast<int>(std::ssize(ptr_fix_up)) + 1] = scene_obj;
    discover_resource_references(scene_obj_node["properties"], type);
  }

  // Preload all resources used by the new scene objects

  std::ranges::sort(required_resource_guids, [](Guid const& lhs, Guid const& rhs) {
    return lhs < rhs;
  });

  required_resource_guids.erase(std::ranges::unique(required_resource_guids, [](Guid const& lhs, Guid const& rhs) {
    return lhs == rhs;
  }).begin(), required_resource_guids.end());

  std::vector<ObserverPtr<Job>> resource_loading_jobs;
  resource_loading_jobs.reserve(required_resource_guids.size());

  for (auto const& guid : required_resource_guids) {
    resource_loading_jobs.emplace_back(App::Instance().GetJobSystem().CreateJob([](Guid const& target_guid) {
      App::Instance().GetResourceManager().GetOrLoad<Resource>(target_guid);
    }, guid));

    App::Instance().GetJobSystem().Run(resource_loading_jobs.back());
  }

  // Deserialize the scene objects

  auto const deserialize_scene_obj_ptr{
    [](YAML::Node const& objNode, rttr::variant& v) -> void {
      if (v.get_type().is_pointer() && v.get_type().get_raw_type().is_derived_from(rttr::type::get<SceneObject>())) {
        if (auto const it{ptr_fix_up.find(objNode.as<int>(0))}; it != std::end(ptr_fix_up)) {
          auto const type{v.get_type()};
          v = it->second;
          [[maybe_unused]] auto const success{v.convert(type)};
          assert(success);
        }
      }
    }
  };

  for (auto const& [fileId, obj] : ptr_fix_up) {
    ReflectionDeserializeFromYaml(yaml_data_["sceneObjects"][fileId - 1]["properties"], *obj,
      deserialize_scene_obj_ptr);
  }

  if (skybox_job) {
    App::Instance().GetJobSystem().Wait(skybox_job);
    skybox_ = skybox_job_data.cubemap;
  }

  for (auto const job : resource_loading_jobs) {
    App::Instance().GetJobSystem().Wait(job);
  }

  // Add the new scene objects to the scene

  for (auto* const scene_obj : scene_objects) {
    if (auto* const entity{rttr::rttr_cast<Entity*>(scene_obj)}) {
      AddEntity(std::unique_ptr<Entity>{entity});
    }
  }
}


auto Scene::SetActive() -> void {
  active_scene_ = this;
}


auto Scene::Clear() -> void {
  for (auto const& entity : entities_) {
    entity->OnBeforeExitingScene(*this);
  }

  entities_.clear();
}


auto Scene::GetAmbientLightVector() const noexcept -> Vector3 const& {
  return ambient_light_;
}


auto Scene::SetAmbientLightVector(Vector3 const& vector) noexcept -> void {
  ambient_light_ = vector;
}


auto Scene::GetAmbientLight() const noexcept -> Color {
  return Color{Vector4{ambient_light_, 1}};
}


auto Scene::SetAmbientLight(Color const& color) noexcept -> void {
  ambient_light_ = Vector3{static_cast<Vector4>(color)};
}


auto Scene::GetSkyMode() const noexcept -> SkyMode {
  return sky_mode_;
}


auto Scene::SetSkyMode(SkyMode const skyMode) noexcept -> void {
  sky_mode_ = skyMode;
}


auto Scene::GetSkyColor() const noexcept -> Vector3 const& {
  return sky_color_;
}


auto Scene::SetSkyColor(Vector3 const& skyColor) noexcept -> void {
  sky_color_ = skyColor;
}


auto Scene::GetSkybox() const noexcept -> Cubemap* {
  return skybox_;
}


auto Scene::SetSkybox(Cubemap* const skybox) noexcept -> void {
  skybox_ = skybox;
  irradiance_map_ = nullptr;
}
}
