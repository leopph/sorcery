#include "sorcery/assets/Material.hpp"

#include <array>
#include <cassert>

#include <spdlog/spdlog.h>

#include "sorcery/asset_reference.hpp"
#include "sorcery/util.hpp"
#include "sorcery/host/app.hpp"
#include "sorcery/host/job_system.hpp"
#include "sorcery/host/assets/asset_manager.hpp"
#include "sorcery/host/assets/material_asset.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{"Material"}
    .property("base_color", &sorcery::Material::GetBaseColorVector, &sorcery::Material::SetBaseColorVector)
    .property("metallic", &sorcery::Material::GetMetallic, &sorcery::Material::SetMetallic)
    .property("roughness", &sorcery::Material::GetRoughness, &sorcery::Material::SetRoughness)
    .property("ao", &sorcery::Material::GetAo, &sorcery::Material::SetAo)
    .property("emissive", &sorcery::Material::GetEmissiveColorVector, &sorcery::Material::SetEmissiveColorVector)
    .property("emissive_intensity", &sorcery::Material::GetEmissiveIntensity, &sorcery::Material::SetEmissiveIntensity)
    .property("base_color_map", &sorcery::Material::GetBaseColorMap, &sorcery::Material::SetBaseColorMapRefl)
    .property("metallicMap", &sorcery::Material::GetMetallicMap, &sorcery::Material::SetMetallicMapRefl)
    .property("roughnessMap", &sorcery::Material::GetRoughnessMap, &sorcery::Material::SetRoughnessMapRefl)
    .property("aoMap", &sorcery::Material::GetAoMap, &sorcery::Material::SetAoMapRefl)
    .property("normalMap", &sorcery::Material::GetNormalMap, &sorcery::Material::SetNormalMapRefl)
    .property("emissive_map", &sorcery::Material::GetEmissiveMap, &sorcery::Material::SetEmissiveMapRefl);
}


namespace sorcery {
namespace {
[[nodiscard]]
auto ToColor(Vector3 const& v) -> Color {
  auto const mul_color_vec = v * 255;
  return Color{
    static_cast<std::uint8_t>(mul_color_vec[0]), static_cast<std::uint8_t>(mul_color_vec[1]),
    static_cast<std::uint8_t>(mul_color_vec[2]), 255
  };
}


[[nodiscard]]
auto ToVector(Color const c) -> Vector3 {
  return Vector3{
    static_cast<float>(c.red) / 255.f, static_cast<float>(c.green) / 255.f,
    static_cast<float>(c.blue) / 255.f
  };
}
}


auto Material::Serialize() const noexcept -> YAML::Node {
  auto const get_asset_id = [](AssetRef<Texture2D> const& tex) -> AssetId {
    return tex ? tex->GetAssetId() : AssetId::Invalid();
  };

  return SerializeMaterialAssetData(MaterialAssetData{
    .base_color = GetBaseColorVector(),
    .metallic = GetMetallic(),
    .roughness = GetRoughness(),
    .ao = GetAo(),
    .emissive = GetEmissiveColorVector(),
    .emissive_intensity = GetEmissiveIntensity(),
    .blend_mode = GetBlendMode(),
    .alpha_threshold = GetAlphaThreshold(),
    .base_color_map = get_asset_id(GetBaseColorMap()),
    .metallic_map = get_asset_id(GetMetallicMap()),
    .roughness_map = get_asset_id(GetRoughnessMap()),
    .ao_map = get_asset_id(GetAoMap()),
    .normal_map = get_asset_id(GetNormalMap()),
    .opacity_map = get_asset_id(GetOpacityMask()),
    .emissive_map = get_asset_id(GetEmissiveMap())
  }, AssetRefSerialization::kGlobal);
}


auto Material::Deserialize(YAML::Node const& yaml_node, YamlDeserializeContext const& ctx) noexcept -> void {
  auto const data{DeserializeMaterialAssetData(yaml_node, ctx)};

  if (!data) {
    spdlog::error("Failed to deserialize material asset data.");
    assert("Failed to deserialize material asset data!" && false);
    return;
  }

  SetBaseColorVector(data->base_color);
  SetMetallic(data->metallic);
  SetRoughness(data->roughness);
  SetAo(data->ao);
  SetEmissiveColorVector(data->emissive);
  SetEmissiveIntensity(data->emissive_intensity);
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

  JobData base_color_map_job_data{};
  JobData metallic_map_job_data{};
  JobData roughness_map_job_data{};
  JobData ao_map_job_data{};
  JobData normal_map_job_data{};
  JobData opacity_mask_job_data{};
  JobData emissive_map_job_data{};

  std::array<ObserverPtr<Job>, 7> jobs{};
  auto job_count = 0uz;

  auto& job_system = App::Instance().GetJobSystem();

  if (data->base_color_map.IsValid()) {
    base_color_map_job_data.asset_id = data->base_color_map;

    auto& base_color_map_job = jobs[job_count];
    base_color_map_job = job_system.CreateJob(loader_job_func, &base_color_map_job_data);
    job_system.Submit(base_color_map_job);

    ++job_count;
  }

  if (data->metallic_map.IsValid()) {
    metallic_map_job_data.asset_id = data->metallic_map;

    auto& metallic_map_job = jobs[job_count];
    metallic_map_job = job_system.CreateJob(loader_job_func, &metallic_map_job_data);
    job_system.Submit(metallic_map_job);

    ++job_count;
  }

  if (data->roughness_map.IsValid()) {
    roughness_map_job_data.asset_id = data->roughness_map;

    auto& roughness_map_job = jobs[job_count];
    roughness_map_job = job_system.CreateJob(loader_job_func, &roughness_map_job_data);
    job_system.Submit(roughness_map_job);

    ++job_count;
  }

  if (data->ao_map.IsValid()) {
    ao_map_job_data.asset_id = data->ao_map;

    auto& ao_map_job = jobs[job_count];
    ao_map_job = job_system.CreateJob(loader_job_func, &ao_map_job_data);
    job_system.Submit(ao_map_job);

    ++job_count;
  }

  if (data->normal_map.IsValid()) {
    normal_map_job_data.asset_id = data->normal_map;

    auto& normal_map_job = jobs[job_count];
    normal_map_job = job_system.CreateJob(loader_job_func, &normal_map_job_data);
    job_system.Submit(normal_map_job);

    ++job_count;
  }

  if (data->opacity_map.IsValid()) {
    opacity_mask_job_data.asset_id = data->opacity_map;

    auto& opacity_mask_job = jobs[job_count];
    opacity_mask_job = job_system.CreateJob(loader_job_func, &opacity_mask_job_data);
    job_system.Submit(opacity_mask_job);

    ++job_count;
  }

  if (data->emissive_map.IsValid()) {
    emissive_map_job_data.asset_id = data->emissive_map;

    auto& emissive_map_job = jobs[job_count];
    emissive_map_job = job_system.CreateJob(loader_job_func, &emissive_map_job_data);
    job_system.Submit(emissive_map_job);

    ++job_count;
  }

  LogException("Failed to load material texture", job_system.TryWaitAll(std::span{jobs}.first(job_count)));

  SetBaseColorMap(base_color_map_job_data.tex);
  SetMetallicMap(metallic_map_job_data.tex);
  SetRoughnessMap(roughness_map_job_data.tex);
  SetAoMap(ao_map_job_data.tex);
  SetNormalMap(normal_map_job_data.tex);
  SetOpacityMask(opacity_mask_job_data.tex);
  SetEmissiveMap(emissive_map_job_data.tex);
}


auto Material::GetBaseColorVector() const -> Vector3 const& {
  return base_color_;
}


auto Material::SetBaseColorVector(Vector3 const& base_color_vector) -> void {
  base_color_ = base_color_vector;
  ++revision_;
}


auto Material::GetBaseColor() const -> Color {
  return ToColor(GetBaseColorVector());
}


auto Material::SetBaseColor(Color const base_color) -> void {
  SetBaseColorVector(ToVector(base_color));
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


auto Material::GetEmissiveColorVector() const -> Vector3 const& {
  return emissive_;
}


auto Material::SetEmissiveColorVector(Vector3 const& emissive_color_vector) -> void {
  emissive_ = emissive_color_vector;
  ++revision_;
}


auto Material::GetEmissiveColor() const -> Color {
  return ToColor(GetEmissiveColorVector());
}


auto Material::SetEmissiveColor(Color const emissive_color) -> void {
  SetEmissiveColorVector(ToVector(emissive_color));
}


auto Material::GetEmissiveIntensity() const -> float {
  return emissive_intensity_;
}


auto Material::SetEmissiveIntensity(float const intensity) -> void {
  if (intensity >= 0) {
    emissive_intensity_ = intensity;
    ++revision_;
  }
}


auto Material::GetBaseColorMap() const -> AssetRef<Texture2D> {
  return base_color_map_;
}


auto Material::SetBaseColorMap(AssetRef<Texture2D> const& tex) -> void {
  base_color_map_ = tex;
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


auto Material::GetEmissiveMap() const -> AssetRef<Texture2D> {
  return emissive_map_;
}


auto Material::SetEmissiveMap(AssetRef<Texture2D> const& tex) -> void {
  emissive_map_ = tex;
  ++revision_;
}


auto Material::GetRevision() const -> std::uint64_t {
  return revision_;
}


// ReSharper disable CppPassValueParameterByConstReference
// Reflection needs getter and setter to be the same type so no ref here
auto Material::SetBaseColorMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetBaseColorMap(tex);
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


auto Material::SetEmissiveMapRefl(AssetRef<Texture2D> const tex) -> void {
  SetEmissiveMap(tex);
}


// ReSharper restore CppPassValueParameterByConstReference
}
