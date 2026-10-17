#include "sorcery/assets/Material.hpp"

#include <array>
#include <cassert>
#include <cstddef>

#include <spdlog/spdlog.h>

#include "sorcery/asset_reference.hpp"
#include "sorcery/util.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/host/assets/material_asset.hpp"


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
  auto const get_asset_id = [](AssetRef<Texture2D> const& tex) -> AssetId {
    return tex ? tex->GetAssetId() : AssetId::Invalid();
  };

  return SerializeMaterialAssetData(MaterialAssetData{
    .base_color = GetAlbedoVector(),
    .metallic = GetMetallic(),
    .roughness = GetRoughness(),
    .ao = GetAo(),
    .blend_mode = GetBlendMode(),
    .alpha_threshold = GetAlphaThreshold(),
    .base_color_map = get_asset_id(GetAlbedoMap()),
    .metallic_map = get_asset_id(GetMetallicMap()),
    .roughness_map = get_asset_id(GetRoughnessMap()),
    .ao_map = get_asset_id(GetAoMap()),
    .normal_map = get_asset_id(GetNormalMap()),
    .opacity_map = get_asset_id(GetOpacityMask())
  }, AssetRefSerialization::kGlobal);
}


auto Material::Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void {
  auto const data{DeserializeMaterialAssetData(yaml_node, ctx)};

  if (!data) {
    spdlog::error("Failed to deserialize material asset data.");
    assert("Failed to deserialize material asset data!" && false);
    return;
  }

  SetAlbedoVector(data->base_color);
  SetMetallic(data->metallic);
  SetRoughness(data->roughness);
  SetAo(data->ao);
  SetBlendMode(data->blend_mode);
  SetAlphaThreshold(data->alpha_threshold);

  struct JobData {
    AssetId asset_id;
    AssetRef<Texture2D> tex;
  };

  auto const loader_job_func{
    [](JobData* const job_data) {
      job_data->tex = MakeAssetRef(App::Instance().GetAssetManager().Resolve<Texture2D>(job_data->asset_id).Get());
    }
  };

  JobData albedo_map_job_data{};
  JobData metallic_map_job_data{};
  JobData roughness_map_job_data{};
  JobData ao_map_job_data{};
  JobData normal_map_job_data{};
  JobData opacity_mask_job_data{};

  std::array<ObserverPtr<Job>, 6> jobs{};
  auto job_count = 0uz;

  if (data->base_color_map.IsValid()) {
    albedo_map_job_data.asset_id = data->base_color_map;

    auto& albedo_map_job = jobs[job_count];
    albedo_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &albedo_map_job_data);
    App::Instance().GetJobSystem().Submit(albedo_map_job);

    ++job_count;
  }

  if (data->metallic_map.IsValid()) {
    metallic_map_job_data.asset_id = data->metallic_map;

    auto& metallic_map_job = jobs[job_count];
    metallic_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &metallic_map_job_data);
    App::Instance().GetJobSystem().Submit(metallic_map_job);

    ++job_count;
  }

  if (data->roughness_map.IsValid()) {
    roughness_map_job_data.asset_id = data->roughness_map;

    auto& roughness_map_job = jobs[job_count];
    roughness_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &roughness_map_job_data);
    App::Instance().GetJobSystem().Submit(roughness_map_job);

    ++job_count;
  }

  if (data->ao_map.IsValid()) {
    ao_map_job_data.asset_id = data->ao_map;

    auto& ao_map_job = jobs[job_count];
    ao_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &ao_map_job_data);
    App::Instance().GetJobSystem().Submit(ao_map_job);

    ++job_count;
  }

  if (data->normal_map.IsValid()) {
    normal_map_job_data.asset_id = data->normal_map;

    auto& normal_map_job = jobs[job_count];
    normal_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &normal_map_job_data);
    App::Instance().GetJobSystem().Submit(normal_map_job);

    ++job_count;
  }

  if (data->opacity_map.IsValid()) {
    opacity_mask_job_data.asset_id = data->opacity_map;

    auto& opacity_mask_job = jobs[job_count];
    opacity_mask_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &opacity_mask_job_data);
    App::Instance().GetJobSystem().Submit(opacity_mask_job);

    ++job_count;
  }

  LogException("Failed to load material texture",
    App::Instance().GetJobSystem().TryWaitAll(std::span{jobs}.first(job_count)));

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


auto Material::GetAlbedoMap() const -> AssetRef<Texture2D> {
  return albedo_map_;
}


auto Material::SetAlbedoMap(AssetRef<Texture2D> const& tex) -> void {
  albedo_map_ = tex;
  ++revision_;
}


auto Material::GetMetallicMap() const -> AssetRef<Texture2D> {
  return metallic_map_;
}


auto Material::SetMetallicMap(AssetRef<Texture2D> const& tex) -> void {
  metallic_map_ = tex;
  ++revision_;
}


auto Material::GetRoughnessMap() const -> AssetRef<Texture2D> {
  return roughness_map_;
}


auto Material::SetRoughnessMap(AssetRef<Texture2D> const& tex) -> void {
  roughness_map_ = tex;
  ++revision_;
}


auto Material::GetAoMap() const -> AssetRef<Texture2D> {
  return ao_map_;
}


auto Material::SetAoMap(AssetRef<Texture2D> const& tex) -> void {
  ao_map_ = tex;
  ++revision_;
}


auto Material::GetNormalMap() const -> AssetRef<Texture2D> {
  return normal_map_;
}


auto Material::SetNormalMap(AssetRef<Texture2D> const& tex) -> void {
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


auto Material::GetOpacityMask() const -> AssetRef<Texture2D> {
  return opacity_mask_;
}


auto Material::SetOpacityMask(AssetRef<Texture2D> const& opacity_mask) -> void {
  opacity_mask_ = opacity_mask;
  ++revision_;
}


auto Material::GetRevision() const -> std::uint64_t {
  return revision_;
}


// ReSharper disable CppPassValueParameterByConstReference
// Reflection needs getter and setter to be the same type so no ref here
auto Material::SetAlbedoMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetAlbedoMap(tex);
}


auto Material::SetMetallicMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetMetallicMap(tex);
}


auto Material::SetRoughnessMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetRoughnessMap(tex);
}


auto Material::SetAoMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetAoMap(tex);
}


auto Material::SetNormalMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetNormalMap(tex);
}


auto Material::SetBlendModeRefl(MaterialBlendMode const blend_mode) -> void {
  SetBlendMode(blend_mode);
}


auto Material::SetAlphaThresholdRefl(float const threshold) -> void {
  SetAlphaThreshold(threshold);
}


auto Material::SetOpacityMaskRefl(AssetRef<Texture2D> const opacity_mask) -> void {
  SetOpacityMask(opacity_mask);
}


// ReSharper restore CppPassValueParameterByConstReference
}
