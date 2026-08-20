#include "Material.hpp"

#include <bit>
#include <cassert>

#include "../app.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../job_system.hpp"
#include "../material_resource.hpp"
#include "../resource_manager.hpp"
#include "../resource_reference.hpp"
#include "../rendering/render_manager.hpp"


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{"Material"}.
    property("albedo", &sorcery::Material::GetAlbedoVector, &sorcery::Material::SetAlbedoVector).
    property("metallic", &sorcery::Material::GetMetallic, &sorcery::Material::SetMetallic).
    property("roughness", &sorcery::Material::GetRoughness, &sorcery::Material::SetRoughness).
    property("ao", &sorcery::Material::GetAo, &sorcery::Material::SetAo).
    property("albedoMap", &sorcery::Material::GetAlbedoMap, &sorcery::Material::SetAlbedoMap).property("metallicMap",
      &sorcery::Material::GetMetallicMap, &sorcery::Material::SetMetallicMap).property("roughnessMap",
      &sorcery::Material::GetRoughnessMap, &sorcery::Material::SetRoughnessMap).property("aoMap",
      &sorcery::Material::GetAoMap, &sorcery::Material::SetAoMap).property("normalMap",
      &sorcery::Material::GetNormalMap, &sorcery::Material::SetNormalMap);
}


namespace sorcery {
auto Material::Serialize() const noexcept -> YAML::Node {
  auto const get_res_id = [](Texture2D const* const tex) -> ResourceId {
    return tex ? tex->GetId() : ResourceId::Invalid();
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
    // TODO log or something?
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
    Texture2D* tex;
  };

  auto const loader_job_func{
    [](JobData* const data) {
      data->tex = App::Instance().GetResourceManager().GetOrLoad<Texture2D>(data->res_id);
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


Material::Material() :
  cb_{rendering::ConstantBuffer<ShaderMaterial>::New(App::Instance().GetGraphicsDevice(), false).value()} {
  Update();
}


Material::~Material() {
  App::Instance().GetRenderManager().KeepAliveWhileInUse(cb_.GetBuffer());
}


auto Material::GetAlbedoVector() const noexcept -> Vector3 const& {
  return mShaderMtl.albedo;
}


auto Material::SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void {
  mShaderMtl.albedo = albedoVector;
  Update();
}


auto Material::GetAlbedoColor() const noexcept -> Color {
  auto const mulColorVec{GetAlbedoVector() * 255};
  return Color{
    static_cast<std::uint8_t>(mulColorVec[0]), static_cast<std::uint8_t>(mulColorVec[1]),
    static_cast<std::uint8_t>(mulColorVec[2]), static_cast<std::uint8_t>(mulColorVec[3])
  };
}


auto Material::SetAlbedoColor(Color const albedoColor) noexcept -> void {
  SetAlbedoVector(Vector3{
    static_cast<float>(albedoColor.red) / 255.f, static_cast<float>(albedoColor.green) / 255.f,
    static_cast<float>(albedoColor.blue) / 255.f
  });
}


auto Material::GetMetallic() const noexcept -> f32 {
  return mShaderMtl.metallic;
}


auto Material::SetMetallic(f32 const metallic) noexcept -> void {
  mShaderMtl.metallic = metallic;
  Update();
}


auto Material::GetRoughness() const noexcept -> f32 {
  return mShaderMtl.roughness;
}


auto Material::SetRoughness(f32 const roughness) noexcept -> void {
  mShaderMtl.roughness = roughness;
  Update();
}


auto Material::GetAo() const noexcept -> f32 {
  return mShaderMtl.ao;
}


auto Material::SetAo(f32 const ao) noexcept -> void {
  mShaderMtl.ao = ao;
  Update();
}


auto Material::GetAlbedoMap() const noexcept -> Texture2D* {
  return albedo_map_;
}


auto Material::SetAlbedoMap(Texture2D* const tex) noexcept -> void {
  albedo_map_ = tex;
  mShaderMtl.albedo_map_idx = albedo_map_ ? albedo_map_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::GetMetallicMap() const noexcept -> Texture2D* {
  return metallic_map_;
}


auto Material::SetMetallicMap(Texture2D* const tex) noexcept -> void {
  metallic_map_ = tex;
  mShaderMtl.metallic_map_idx = metallic_map_ ? metallic_map_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::GetRoughnessMap() const noexcept -> Texture2D* {
  return roughness_map_;
}


auto Material::SetRoughnessMap(Texture2D* const tex) noexcept -> void {
  roughness_map_ = tex;
  mShaderMtl.roughness_map_idx = roughness_map_ ? roughness_map_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::GetAoMap() const noexcept -> Texture2D* {
  return ao_map_;
}


auto Material::SetAoMap(Texture2D* const tex) noexcept -> void {
  ao_map_ = tex;
  mShaderMtl.ao_map_idx = ao_map_ ? ao_map_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::GetNormalMap() const noexcept -> Texture2D* {
  return normal_map_;
}


auto Material::SetNormalMap(Texture2D* const tex) noexcept -> void {
  normal_map_ = tex;
  mShaderMtl.normal_map_idx = normal_map_ ? normal_map_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::GetBlendMode() const noexcept -> BlendMode {
  return static_cast<BlendMode>(mShaderMtl.blendMode);
}


auto Material::SetBlendMode(BlendMode blendMode) noexcept -> void {
  mShaderMtl.blendMode = static_cast<int>(blendMode);
}


auto Material::GetAlphaThreshold() const noexcept -> float {
  return mShaderMtl.alphaThreshold;
}


auto Material::SetAlphaThreshold(float const threshold) noexcept -> void {
  mShaderMtl.alphaThreshold = threshold;
  Update();
}


auto Material::GetOpacityMask() const noexcept -> Texture2D* {
  return opacity_mask_;
}


auto Material::SetOpacityMask(Texture2D* const opacityMask) noexcept -> void {
  opacity_mask_ = opacityMask;
  mShaderMtl.opacity_map_idx = opacity_mask_ ? opacity_mask_->GetTex()->GetShaderResource() : INVALID_RES_IDX;
  Update();
}


auto Material::Update() const -> void {
  App::Instance().GetRenderManager().UpdateBuffer(*cb_.GetBuffer(), 0, std::span{
    std::bit_cast<std::byte const*>(&mShaderMtl), sizeof(mShaderMtl)
  });
}


auto Material::GetBuffer() const noexcept -> graphics::SharedDeviceChildHandle<graphics::Buffer> const& {
  return cb_.GetBuffer();
}
}
