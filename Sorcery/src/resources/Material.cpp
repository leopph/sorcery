#include "Material.hpp"

#include "../app.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../GUI.hpp"
#include "../job_system.hpp"
#include "../ResourceManager.hpp"
#include "../rendering/render_manager.hpp"

#include <imgui.h>

#include <bit>
#include <cstdint>
#include <tuple>


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


auto Material::Serialize() const noexcept -> YAML::Node {
  YAML::Node ret;

  ret["albedo"] = GetAlbedoVector();
  ret["metallic"] = GetMetallic();
  ret["roughness"] = GetRoughness();
  ret["ao"] = GetAo();
  ret["blendMode"] = static_cast<int>(GetBlendMode());
  ret["alphaThresh"] = GetAlphaThreshold();

  auto const albedoMap{GetAlbedoMap()};
  ret["albedoMap"] = albedoMap ? albedoMap->GetGuid() : Guid::Invalid();

  auto const metallicMap{GetMetallicMap()};
  ret["metallicMap"] = metallicMap ? metallicMap->GetGuid() : Guid::Invalid();

  auto const roughnessMap{GetRoughnessMap()};
  ret["roughnessMap"] = roughnessMap ? roughnessMap->GetGuid() : Guid::Invalid();

  auto const aoMap{GetAoMap()};
  ret["aoMap"] = aoMap ? aoMap->GetGuid() : Guid::Invalid();

  auto const normalMap{GetNormalMap()};
  ret["normalMap"] = normalMap ? normalMap->GetGuid() : Guid::Invalid();

  auto const opacityMask{GetOpacityMask()};
  ret["opacityMask"] = opacityMask ? opacityMask->GetGuid() : Guid::Invalid();

  return ret;
}


auto Material::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  SetAlbedoVector(yamlNode["albedo"].as<Vector3>(GetAlbedoVector()));
  SetMetallic(yamlNode["metallic"].as<float>(GetMetallic()));
  SetRoughness(yamlNode["roughness"].as<float>(GetRoughness()));
  SetAo(yamlNode["ao"].as<float>(GetAo()));
  SetBlendMode(static_cast<BlendMode>(yamlNode["blendMode"].as<int>(static_cast<int>(GetBlendMode()))));
  SetAlphaThreshold(yamlNode["alphaThresh"].as<float>(GetAlphaThreshold()));

  struct JobData {
    Guid guid;
    Texture2D* tex;
  };

  auto const loader_job_func{
    [](void const* const data_ptr) {
      auto& [guid, tex]{**static_cast<JobData* const*>(data_ptr)};
      tex = App::Instance().GetResourceManager().GetOrLoad<Texture2D>(guid);
    }
  };

  Job* albedo_map_job{};
  JobData albedo_map_job_data{};

  Job* metallic_map_job{};
  JobData metallic_map_job_data{};

  Job* roughness_map_job{};
  JobData roughness_map_job_data{};

  Job* ao_map_job{};
  JobData ao_map_job_data{};

  Job* normal_map_job{};
  JobData normal_map_job_data{};

  Job* opacity_mask_job{};
  JobData opacity_mask_job_data{};

  if (auto const guid{yamlNode["albedoMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    albedo_map_job_data.guid = guid;
    albedo_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &albedo_map_job_data);
    App::Instance().GetJobSystem().Run(albedo_map_job);
  }

  if (auto const guid{yamlNode["metallicMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    metallic_map_job_data.guid = guid;
    metallic_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &metallic_map_job_data);
    App::Instance().GetJobSystem().Run(metallic_map_job);
  }

  if (auto const guid{yamlNode["roughnessMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    roughness_map_job_data.guid = guid;
    roughness_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &roughness_map_job_data);
    App::Instance().GetJobSystem().Run(roughness_map_job);
  }

  if (auto const guid{yamlNode["aoMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    ao_map_job_data.guid = guid;
    ao_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &ao_map_job_data);
    App::Instance().GetJobSystem().Run(ao_map_job);
  }

  if (auto const guid{yamlNode["normalMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    normal_map_job_data.guid = guid;
    normal_map_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &normal_map_job_data);
    App::Instance().GetJobSystem().Run(normal_map_job);
  }

  if (auto const guid{yamlNode["opacityMask"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    opacity_mask_job_data.guid = guid;
    opacity_mask_job = App::Instance().GetJobSystem().CreateJob(loader_job_func, &opacity_mask_job_data);
    App::Instance().GetJobSystem().Run(opacity_mask_job);
  }

  for (auto const* const job : {
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


auto Material::OnDrawProperties(bool& changed) -> void {
  NativeResource::OnDrawProperties(changed);

  if (ImGui::BeginTable(std::format("{}", GetGuid().ToString()).c_str(), 2, ImGuiTableFlags_SizingStretchSame)) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::PushItemWidth(FLT_MIN);
    ImGui::TableSetColumnIndex(1);
    ImGui::PushItemWidth(-FLT_MIN);

    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Albedo Color");
    ImGui::TableNextColumn();

    if (Vector3 albedoColor{GetAlbedoVector()}; ImGui::ColorEdit3("##matAlbedoColor", albedoColor.GetData())) {
      SetAlbedoVector(albedoColor);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Metallic");
    ImGui::TableNextColumn();

    if (f32 metallic{GetMetallic()}; ImGui::SliderFloat("##matMetallic", &metallic, 0.0f, 1.0f)) {
      SetMetallic(metallic);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Roughness");
    ImGui::TableNextColumn();

    if (f32 roughness{GetRoughness()}; ImGui::SliderFloat("##matRoughness", &roughness, 0.0f, 1.0f)) {
      SetRoughness(roughness);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Ambient Occlusion");
    ImGui::TableNextColumn();

    if (f32 ao{GetAo()}; ImGui::SliderFloat("##matAo", &ao, 0.0f, 1.0f)) {
      SetAo(ao);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Albedo Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> albedoMapPicker;
    if (auto albedoMap{GetAlbedoMap()}; albedoMapPicker.Draw(albedoMap)) {
      SetAlbedoMap(albedoMap);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Metallic Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> metallicMapPicker;
    if (auto metallicMap{GetMetallicMap()}; metallicMapPicker.Draw(metallicMap)) {
      SetMetallicMap(metallicMap);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Roughness Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> roughnessMapPicker;
    if (auto roughnessMap{GetRoughnessMap()}; roughnessMapPicker.Draw(roughnessMap)) {
      SetRoughnessMap(roughnessMap);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Ambient Occlusion Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> aoMapPicker;
    if (auto aoMap{GetAoMap()}; aoMapPicker.Draw(aoMap)) {
      SetAoMap(aoMap);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Normal Map");
    ImGui::TableNextColumn();
    static ObjectPicker<Texture2D> normalMapPicker;
    if (auto normalMap{GetNormalMap()}; normalMapPicker.Draw(normalMap)) {
      SetNormalMap(normalMap);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("%s", "Blend Mode");
    ImGui::TableNextColumn();
    if (char const* blendModeNames[]{"Opaque", "Alpha Clipping"}; ImGui::BeginCombo("##blendMode",
      blendModeNames[static_cast<int>(GetBlendMode())])) {
      for (int i = 0; i < 2; i++) {
        if (ImGui::Selectable(blendModeNames[i], i == static_cast<int>(GetBlendMode()))) {
          SetBlendMode(static_cast<BlendMode>(i));
          changed = true;
        }
      }
      ImGui::EndCombo();
    }

    if (GetBlendMode() == BlendMode::AlphaClip) {
      ImGui::TableNextColumn();
      ImGui::Text("%s", "Alpha Threshold");
      ImGui::TableNextColumn();
      if (auto thresh{GetAlphaThreshold()}; ImGui::SliderFloat("##AlphaThresh", &thresh, 0, 1)) {
        SetAlphaThreshold(thresh);
        changed = true;
      }

      ImGui::TableNextColumn();
      ImGui::Text("%s", "Opacity Mask");
      ImGui::TableNextColumn();
      static ObjectPicker<Texture2D> opacityMaskPicker;
      if (auto opacityMask{GetOpacityMask()}; opacityMaskPicker.Draw(opacityMask)) {
        SetOpacityMask(opacityMask);
        changed = true;
      }
    }

    ImGui::EndTable();
  }
}
}
