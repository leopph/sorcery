#include "Material.hpp"

#include <cassert>

#include <spdlog/spdlog.h>

#include "../app.hpp"
#include "../job_system.hpp"
#include "../material_resource.hpp"
#include "../resource_manager.hpp"
#include "../resource_reference.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{"Material"}
    .property("albedo", &sorcery::Material::GetAlbedoVector, &sorcery::Material::SetAlbedoVector)
    .property("metallic", &sorcery::Material::GetMetallic, &sorcery::Material::SetMetallic)
    .property("roughness", &sorcery::Material::GetRoughness, &sorcery::Material::SetRoughness)
    .property("ao", &sorcery::Material::GetAo, &sorcery::Material::SetAo)
    .property("albedoMap", &sorcery::Material::GetAlbedoMap, &sorcery::Material::SetAlbedoMapRefl)
    .property("metallicMap", &sorcery::Material::GetMetallicMap, &sorcery::Material::SetMetallicMapRefl)
    .property("roughnessMap", &sorcery::Material::GetRoughnessMap, &sorcery::Material::SetRoughnessMapRefl)
    .property("aoMap", &sorcery::Material::GetAoMap, &sorcery::Material::SetAoMapRefl)
    .property("normalMap", &sorcery::Material::GetNormalMap, &sorcery::Material::SetNormalMapRefl);
}


namespace sorcery {
auto Material::Serialize() const noexcept -> YAML::Node {
  auto const get_res_id = [](ResourceRef<Texture2D> const& tex) -> ResourceId {
    return tex ? tex->GetResId() : ResourceId::Invalid();
  };

  return SerializeMaterialResourceData(MaterialResourceData{
    .base_color = GetAlbedoVector(),
    .metallic = GetMetallic(),
    .roughness = GetRoughness(),
    .ao = GetAo(),
    .blend_mode = GetBlendMode(),
    .alpha_threshold = GetAlphaThreshold(),
    .base_color_map = get_res_id(GetAlbedoMap()),
    .metallic_map = get_res_id(GetMetallicMap()),
    .roughness_map = get_res_id(GetRoughnessMap()),
    .ao_map = get_res_id(GetAoMap()),
    .normal_map = get_res_id(GetNormalMap()),
    .opacity_map = get_res_id(GetOpacityMask())
  }, ResourceRefSerialization::kGlobal);
}


auto Material::Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void {
  auto const data{DeserializeMaterialResourceData(yaml_node, ctx)};

  if (!data) {
    spdlog::error("Failed to deserialize material resource data.");
    assert("Failed to deserialize material resource data!" && false);
    return;
  }

  SetAlbedoVector(data->base_color);
  SetMetallic(data->metallic);
  SetRoughness(data->roughness);
  SetAo(data->ao);
  SetBlendMode(data->blend_mode);
  SetAlphaThreshold(data->alpha_threshold);

  struct JobData {
    ResourceId res_id;
    ResourceRef<Texture2D> tex;
  };

  auto const loader_job_func{
    [](JobData* const job_data) {
      job_data->tex = MakeResourceRef(App::Instance().GetResourceManager().Resolve<Texture2D>(job_data->res_id).Get());
    }
  };

  ObserverPtr<Job> albedo_map_job{};
  JobData albedo_map_job_data{};

  ObserverPtr<Job> metallic_map_job{};
  JobData metallic_map_job_data{};

  ObserverPtr<Job> roughness_map_job{};
  JobData roughness_map_job_data{};

  ObserverPtr<Job> ao_map_job{};
  JobData ao_map_job_data{};

  ObserverPtr<Job> normal_map_job{};
  JobData normal_map_job_data{};

  ObserverPtr<Job> opacity_mask_job{};
  JobData opacity_mask_job_data{};

  if (data->base_color_map.IsValid()) {
    albedo_map_job_data.res_id = data->base_color_map;
    albedo_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &albedo_map_job_data);
    App::Instance().GetJobSystem().Run(albedo_map_job);
  }

  if (data->metallic_map.IsValid()) {
    metallic_map_job_data.res_id = data->metallic_map;
    metallic_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &metallic_map_job_data);
    App::Instance().GetJobSystem().Run(metallic_map_job);
  }

  if (data->roughness_map.IsValid()) {
    roughness_map_job_data.res_id = data->roughness_map;
    roughness_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &roughness_map_job_data);
    App::Instance().GetJobSystem().Run(roughness_map_job);
  }

  if (data->ao_map.IsValid()) {
    ao_map_job_data.res_id = data->ao_map;
    ao_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &ao_map_job_data);
    App::Instance().GetJobSystem().Run(ao_map_job);
  }

  if (data->normal_map.IsValid()) {
    normal_map_job_data.res_id = data->normal_map;
    normal_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &normal_map_job_data);
    App::Instance().GetJobSystem().Run(normal_map_job);
  }

  if (data->opacity_map.IsValid()) {
    opacity_mask_job_data.res_id = data->opacity_map;
    opacity_mask_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &opacity_mask_job_data);
    App::Instance().GetJobSystem().Run(opacity_mask_job);
  }

  for (auto const job : {
         albedo_map_job, metallic_map_job, roughness_map_job, ao_map_job, normal_map_job, opacity_mask_job
       }) {
    if (job) {
      App::Instance().GetJobSystem().Wait(job);
    }
  }

  SetAlbedoMap(albedo_map_job_data.tex);
  SetMetallicMap(metallic_map_job_data.tex);
  SetRoughnessMap(roughness_map_job_data.tex);
  SetAoMap(ao_map_job_data.tex);
  SetNormalMap(normal_map_job_data.tex);
  SetOpacityMask(opacity_mask_job_data.tex);
}


auto Material::GetAlbedoVector() const -> Vector3 const& {
  return albedo_;
}


auto Material::SetAlbedoVector(Vector3 const& albedo_vector) -> void {
  albedo_ = albedo_vector;
  ++revision_;
}


auto Material::GetAlbedoColor() const -> Color {
  auto const mul_color_vec{GetAlbedoVector() * 255};
  return Color{
    static_cast<std::uint8_t>(mul_color_vec[0]), static_cast<std::uint8_t>(mul_color_vec[1]),
    static_cast<std::uint8_t>(mul_color_vec[2]), 255
  };
}


auto Material::SetAlbedoColor(Color const albedo_color) -> void {
  SetAlbedoVector(Vector3{
    static_cast<float>(albedo_color.red) / 255.f, static_cast<float>(albedo_color.green) / 255.f,
    static_cast<float>(albedo_color.blue) / 255.f
  });
}


auto Material::GetMetallic() const -> f32 {
  return metallic_;
}


auto Material::SetMetallic(f32 const metallic) -> void {
  metallic_ = metallic;
  ++revision_;
}


auto Material::GetRoughness() const -> f32 {
  return roughness_;
}


auto Material::SetRoughness(f32 const roughness) -> void {
  roughness_ = roughness;
  ++revision_;
}


auto Material::GetAo() const -> f32 {
  return ao_;
}


auto Material::SetAo(f32 const ao) -> void {
  ao_ = ao;
  ++revision_;
}


auto Material::GetAlbedoMap() const -> ResourceRef<Texture2D> {
  return albedo_map_;
}


auto Material::SetAlbedoMap(ResourceRef<Texture2D> const& tex) -> void {
  albedo_map_ = tex;
  ++revision_;
}


auto Material::GetMetallicMap() const -> ResourceRef<Texture2D> {
  return metallic_map_;
}


auto Material::SetMetallicMap(ResourceRef<Texture2D> const& tex) -> void {
  metallic_map_ = tex;
  ++revision_;
}


auto Material::GetRoughnessMap() const -> ResourceRef<Texture2D> {
  return roughness_map_;
}


auto Material::SetRoughnessMap(ResourceRef<Texture2D> const& tex) -> void {
  roughness_map_ = tex;
  ++revision_;
}


auto Material::GetAoMap() const -> ResourceRef<Texture2D> {
  return ao_map_;
}


auto Material::SetAoMap(ResourceRef<Texture2D> const& tex) -> void {
  ao_map_ = tex;
  ++revision_;
}


auto Material::GetNormalMap() const -> ResourceRef<Texture2D> {
  return normal_map_;
}


auto Material::SetNormalMap(ResourceRef<Texture2D> const& tex) -> void {
  normal_map_ = tex;
  ++revision_;
}


auto Material::GetBlendMode() const -> MaterialBlendMode {
  return blend_mode_;
}


auto Material::SetBlendMode(MaterialBlendMode const blend_mode) -> void {
  blend_mode_ = blend_mode;
  ++revision_;
}


auto Material::GetAlphaThreshold() const -> float {
  return alpha_threshold_;
}


auto Material::SetAlphaThreshold(float const threshold) -> void {
  alpha_threshold_ = threshold;
  ++revision_;
}


auto Material::GetOpacityMask() const -> ResourceRef<Texture2D> {
  return opacity_mask_;
}


auto Material::SetOpacityMask(ResourceRef<Texture2D> const& opacity_mask) -> void {
  opacity_mask_ = opacity_mask;
  ++revision_;
}


auto Material::GetRevision() const -> std::uint64_t {
  return revision_;
}


// ReSharper disable CppPassValueParameterByConstReference
// Reflection needs getter and setter to be the same type so no ref here
auto Material::SetAlbedoMapRefl(ResourceRef<Texture2D> const tex) -> void {
  SetAlbedoMap(tex);
}


auto Material::SetMetallicMapRefl(ResourceRef<Texture2D> const tex) -> void {
  SetMetallicMap(tex);
}


auto Material::SetRoughnessMapRefl(ResourceRef<Texture2D> const tex) -> void {
  SetRoughnessMap(tex);
}


auto Material::SetAoMapRefl(ResourceRef<Texture2D> const tex) -> void {
  SetAoMap(tex);
}


auto Material::SetNormalMapRefl(ResourceRef<Texture2D> const tex) -> void {
  SetNormalMap(tex);
}


auto Material::SetBlendModeRefl(MaterialBlendMode const blend_mode) -> void {
  SetBlendMode(blend_mode);
}


auto Material::SetAlphaThresholdRefl(float const threshold) -> void {
  SetAlphaThreshold(threshold);
}


auto Material::SetOpacityMaskRefl(ResourceRef<Texture2D> const opacity_mask) -> void {
  SetOpacityMask(opacity_mask);
}


// ReSharper restore CppPassValueParameterByConstReference
}
