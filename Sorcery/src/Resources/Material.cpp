#include "Material.hpp"

#include "../Rendering/Renderer.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../GUI.hpp"
#include "../ResourceManager.hpp"

#include <cstdint>
#include <imgui.h>
#include <stdexcept>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{"Material"}
    .property("albedo", &sorcery::Material::GetAlbedoVector, &sorcery::Material::SetAlbedoVector)
    .property("metallic", &sorcery::Material::GetMetallic, &sorcery::Material::SetMetallic)
    .property("roughness", &sorcery::Material::GetRoughness, &sorcery::Material::SetRoughness)
    .property("ao", &sorcery::Material::GetAo, &sorcery::Material::SetAo)
    .property("albedoMap", &sorcery::Material::GetAlbedoMap, &sorcery::Material::SetAlbedoMap)
    .property("metallicMap", &sorcery::Material::GetMetallicMap, &sorcery::Material::SetMetallicMap)
    .property("roughnessMap", &sorcery::Material::GetRoughnessMap, &sorcery::Material::SetRoughnessMap)
    .property("aoMap", &sorcery::Material::GetAoMap, &sorcery::Material::SetAoMap)
    .property("normalMap", &sorcery::Material::GetNormalMap, &sorcery::Material::SetNormalMap);
}


namespace sorcery {
auto Material::UpdateGPUData() const noexcept -> void {
  auto const ctx{gRenderer.GetThreadContext()};

  D3D11_MAPPED_SUBRESOURCE mapped;
  [[maybe_unused]] auto hr{ctx->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)};
  assert(SUCCEEDED(hr));

  *static_cast<ShaderMaterial*>(mapped.pData) = mShaderMtl;

  ctx->Unmap(mCB.Get(), 0);

  Microsoft::WRL::ComPtr<ID3D11CommandList> cmdList;
  hr = ctx->FinishCommandList(FALSE, cmdList.GetAddressOf());
  assert(SUCCEEDED(hr));

  gRenderer.ExecuteCommandList(cmdList.Get());
}


auto Material::CreateCB() -> void {
  D3D11_BUFFER_DESC constexpr cbDesc{
    .ByteWidth = clamp_cast<UINT>(RoundToNextMultiple(sizeof(ShaderMaterial), 16)),
    .Usage = D3D11_USAGE_DYNAMIC,
    .BindFlags = D3D11_BIND_CONSTANT_BUFFER,
    .CPUAccessFlags = D3D11_CPU_ACCESS_WRITE,
    .MiscFlags = 0,
    .StructureByteStride = 0
  };

  D3D11_SUBRESOURCE_DATA const initialData{
    .pSysMem = &mShaderMtl,
    .SysMemPitch = 0,
    .SysMemSlicePitch = 0
  };

  if (FAILED(gRenderer.GetDevice()->CreateBuffer(&cbDesc, &initialData, mCB.GetAddressOf()))) {
    throw std::runtime_error{"Failed to create material CB."};
  }
}


Material::Material() {
  CreateCB();
}


auto Material::GetAlbedoVector() const noexcept -> Vector3 const& {
  return mShaderMtl.albedo;
}


auto Material::SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void {
  mShaderMtl.albedo = albedoVector;
  UpdateGPUData();
}


auto Material::GetAlbedoColor() const noexcept -> Color {
  auto const mulColorVec{GetAlbedoVector() * 255};
  return Color{static_cast<std::uint8_t>(mulColorVec[0]), static_cast<std::uint8_t>(mulColorVec[1]), static_cast<std::uint8_t>(mulColorVec[2]), static_cast<std::uint8_t>(mulColorVec[3])};
}


auto Material::SetAlbedoColor(Color const albedoColor) noexcept -> void {
  SetAlbedoVector(Vector3{static_cast<float>(albedoColor.red) / 255.f, static_cast<float>(albedoColor.green) / 255.f, static_cast<float>(albedoColor.blue) / 255.f});
}


auto Material::GetMetallic() const noexcept -> f32 {
  return mShaderMtl.metallic;
}


auto Material::SetMetallic(f32 const metallic) noexcept -> void {
  mShaderMtl.metallic = metallic;
  UpdateGPUData();
}


auto Material::GetRoughness() const noexcept -> f32 {
  return mShaderMtl.roughness;
}


auto Material::SetRoughness(f32 const roughness) noexcept -> void {
  mShaderMtl.roughness = roughness;
  UpdateGPUData();
}


auto Material::GetAo() const noexcept -> f32 {
  return mShaderMtl.ao;
}


auto Material::SetAo(f32 const ao) noexcept -> void {
  mShaderMtl.ao = ao;
  UpdateGPUData();
}


auto Material::GetAlbedoMap() const noexcept -> ObserverPtr<Texture2D> {
  return mAlbedoMap;
}


auto Material::SetAlbedoMap(ObserverPtr<Texture2D> const tex) noexcept -> void {
  mAlbedoMap = tex;
  mShaderMtl.sampleAlbedo = mAlbedoMap != nullptr;
  UpdateGPUData();
}


auto Material::GetMetallicMap() const noexcept -> ObserverPtr<Texture2D> {
  return mMetallicMap;
}


auto Material::SetMetallicMap(ObserverPtr<Texture2D> const tex) noexcept -> void {
  mMetallicMap = tex;
  mShaderMtl.sampleMetallic = mMetallicMap != nullptr;
  UpdateGPUData();
}


auto Material::GetRoughnessMap() const noexcept -> ObserverPtr<Texture2D> {
  return mRoughnessMap;
}


auto Material::SetRoughnessMap(ObserverPtr<Texture2D> const tex) noexcept -> void {
  mRoughnessMap = tex;
  mShaderMtl.sampleRoughness = mRoughnessMap != nullptr;
  UpdateGPUData();
}


auto Material::GetAoMap() const noexcept -> ObserverPtr<Texture2D> {
  return mAoMap;
}


auto Material::SetAoMap(ObserverPtr<Texture2D> const tex) noexcept -> void {
  mAoMap = tex;
  mShaderMtl.sampleAo = mAoMap != nullptr;
  UpdateGPUData();
}


auto Material::GetNormalMap() const noexcept -> ObserverPtr<Texture2D> {
  return mNormalMap;
}


auto Material::SetNormalMap(ObserverPtr<Texture2D> const tex) noexcept -> void {
  mNormalMap = tex;
  mShaderMtl.sampleNormal = mNormalMap != nullptr;
  UpdateGPUData();
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
  UpdateGPUData();
}


auto Material::GetOpacityMask() const noexcept -> ObserverPtr<Texture2D> {
  return mOpacityMask;
}


auto Material::SetOpacityMask(ObserverPtr<Texture2D> const opacityMask) noexcept -> void {
  mOpacityMask = opacityMask;
  mShaderMtl.sampleOpacityMap = mOpacityMask != nullptr;
  UpdateGPUData();
}


auto Material::GetBuffer() const noexcept -> ObserverPtr<ID3D11Buffer> {
  return mCB.Get();
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

  if (auto const guid{yamlNode["albedoMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetAlbedoMap(gResourceManager.GetOrLoad<Texture2D>(guid));
  }

  if (auto const guid{yamlNode["metallicMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetMetallicMap(gResourceManager.GetOrLoad<Texture2D>(guid));
  }

  if (auto const guid{yamlNode["roughnessMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetRoughnessMap(gResourceManager.GetOrLoad<Texture2D>(guid));
  }

  if (auto const guid{yamlNode["aoMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetAoMap(gResourceManager.GetOrLoad<Texture2D>(guid));
  }

  if (auto const guid{yamlNode["normalMap"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetNormalMap(gResourceManager.GetOrLoad<Texture2D>(guid));
  }

  if (auto const guid{yamlNode["opacityMask"].as<Guid>(Guid::Invalid())}; guid.IsValid()) {
    SetOpacityMask(gResourceManager.GetOrLoad<Texture2D>(guid));
  }
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
    if (char const* blendModeNames[]{"Opaque", "Alpha Clipping"}; ImGui::BeginCombo("##blendMode", blendModeNames[static_cast<int>(GetBlendMode())])) {
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
