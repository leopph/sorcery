#include "Scene.hpp"

#include <algorithm>
#include <ranges>

#include "../app.hpp"
#include "../resource_reference.hpp"
#include "../Serialization.hpp"
#include "../scene_objects/SceneObject.hpp"
#undef FindResource
#include "../entity_serialization.hpp"
#include "../job_system.hpp"
#include "../Reflection.hpp"
#include "../resource_manager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Scene>{"Scene"}
    .property("skybox", &sorcery::Scene::GetSkybox, &sorcery::Scene::SetSkybox);
}


namespace sorcery {
auto detail::GetIrradianceMap(Scene const& scene) -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return scene.irradiance_map_;
}


auto detail::RecreateIrradianceMap(Scene& scene, graphics::GraphicsDevice& device, DXGI_FORMAT const format,
                                   UINT const size) -> void {
  if (!scene.skybox_) {
    return;
  }

  scene.irradiance_map_ = device.CreateTexture(graphics::TextureDesc{
      .dimension = graphics::TextureDimension::kCube,
      .width = size,
      .height = size,
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


auto detail::GetPrefilteredEnvMap(Scene const& scene) -> graphics::SharedDeviceChildHandle<graphics::Texture> const& {
  return scene.prefiltered_env_map_;
}


auto detail::RecreatePrefilteredEnvMap(Scene& scene, graphics::GraphicsDevice& device, DXGI_FORMAT const format,
                                       UINT const size) -> void {
  scene.prefiltered_env_map_ = device.CreateTexture(graphics::TextureDesc{
      .dimension = graphics::TextureDimension::kCube,
      .width = size,
      .height = size,
      .depth_or_array_size = 6,
      .mip_levels = 0,
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
  yaml_data_.reset();
  yaml_data_["version"] = 1;
  yaml_data_["ambientLight"] = ambient_light_;
  yaml_data_["skyMode"] = static_cast<int>(sky_mode_);
  yaml_data_["skyColor"] = sky_color_;
  yaml_data_["skybox"] = SerializeGlobalResourceId(skybox_ ? skybox_->GetId() : ResourceId::Invalid());

  std::vector<Entity const*> entities;
  entities.reserve(entities_.size());
  std::ranges::transform(entities_, std::back_inserter(entities), &std::unique_ptr<Entity>::get);

  yaml_data_["sceneObjects"] = SerializeEntitySet(entities, EntitySerializationContext{
    .resource_ref_serialization = ResourceRefSerialization::kGlobal
  });
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
    ResourceId res_id;
    Cubemap* cubemap;
  } skybox_job_data;

  ObserverPtr<Job> skybox_job;

  if (auto const node{yaml_data_["skybox"]}) {
    if (auto const res_id{DeserializeResourceId(node, yaml_ctx_).value_or(ResourceId::Invalid())}; res_id.IsValid()) {
      skybox_job_data.res_id = res_id;

      skybox_job = App::Instance().GetJobSystem().CreateJob([](SkyboxJobData* const data) {
        data->cubemap = App::Instance().GetResourceManager().GetOrLoad<Cubemap>(data->res_id);
      }, &skybox_job_data);

      App::Instance().GetJobSystem().Run(skybox_job);
    }
  }

  // Add the new entities to the scene

  for (auto& entity_ptr : DeserializeEntitySet(yaml_data_["sceneObjects"], yaml_ctx_)) {
    AddEntity(std::move(entity_ptr));
  }

  if (skybox_job) {
    App::Instance().GetJobSystem().Wait(skybox_job);
    skybox_ = skybox_job_data.cubemap;
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


auto Scene::Serialize() const noexcept -> YAML::Node {
  return yaml_data_;
}


auto Scene::Deserialize(
  YAML::Node const& yaml_node,
  YamlDeserializeContext const& ctx
) noexcept -> void {
  yaml_data_ = Clone(yaml_node);
  yaml_ctx_ = ctx;
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
  prefiltered_env_map_ = nullptr;
}


Scene* Scene::active_scene_{nullptr};


std::vector<Scene*> Scene::all_scenes_;
}
