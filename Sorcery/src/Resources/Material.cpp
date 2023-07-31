#include "Material.hpp"

#include "../Renderer.hpp"
#include "../Serialization.hpp"
#undef FindResource
#include "../ResourceManager.hpp"

#include <cstdint>
#include <imgui.h>
#include <imgui_stdlib.h>
#include <stdexcept>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{ "Material" };
}


namespace sorcery {
auto Material::UpdateGPUData() const noexcept -> void {
  D3D11_MAPPED_SUBRESOURCE mappedCB;
  gRenderer.GetImmediateContext()->Map(mCB.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedCB);
  *static_cast<ShaderMaterial*>(mappedCB.pData) = mShaderMtl;
  gRenderer.GetImmediateContext()->Unmap(mCB.Get(), 0);
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
    throw std::runtime_error{ "Failed to create material CB." };
  }
}


Material::Material() {
  CreateCB();
}


auto Material::GetAlbedoVector() const noexcept -> Vector3 {
  return mShaderMtl.albedo;
}


auto Material::SetAlbedoVector(Vector3 const& albedoVector) noexcept -> void {
  mShaderMtl.albedo = albedoVector;
  UpdateGPUData();
}


auto Material::GetAlbedoColor() const noexcept -> Color {
  auto const mulColorVec{ GetAlbedoVector() * 255 };
  return Color{ static_cast<u8>(mulColorVec[0]), static_cast<u8>(mulColorVec[1]), static_cast<u8>(mulColorVec[2]), static_cast<u8>(mulColorVec[3]) };
}


auto Material::SetAlbedoColor(Color const& albedoColor) noexcept -> void {
  SetAlbedoVector(Vector3{ static_cast<float>(albedoColor.red) / 255.f, static_cast<float>(albedoColor.green) / 255.f, static_cast<float>(albedoColor.blue) / 255.f });
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


auto Material::GetBuffer() const noexcept -> ObserverPtr<ID3D11Buffer> {
  return mCB.Get();
}


auto Material::Serialize() const noexcept -> YAML::Node {
  YAML::Node ret;

  ret["albedo"] = GetAlbedoVector();
  ret["metallic"] = GetMetallic();
  ret["roughness"] = GetRoughness();
  ret["ao"] = GetAo();

  auto const albedoMap{ GetAlbedoMap() };
  ret["albedoMap"] = albedoMap
                       ? albedoMap->GetGuid()
                       : Guid::Invalid();

  auto const metallicMap{ GetMetallicMap() };
  ret["metallicMap"] = metallicMap
                         ? metallicMap->GetGuid()
                         : Guid::Invalid();

  auto const roughnessMap{ GetRoughnessMap() };
  ret["roughnessMap"] = roughnessMap
                          ? roughnessMap->GetGuid()
                          : Guid::Invalid();

  auto const aoMap{ GetAoMap() };
  ret["aoMap"] = aoMap
                   ? aoMap->GetGuid()
                   : Guid::Invalid();

  auto const normalMap{ GetNormalMap() };
  ret["normalMap"] = normalMap
                       ? normalMap->GetGuid()
                       : Guid::Invalid();

  return ret;
}


auto Material::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  SetAlbedoVector(yamlNode["albedo"].as<Vector3>(GetAlbedoVector()));
  SetMetallic(yamlNode["metallic"].as<float>(GetMetallic()));
  SetRoughness(yamlNode["roughness"].as<float>(GetRoughness()));
  SetAo(yamlNode["ao"].as<float>(GetAo()));

  auto const albedoMapGuid{ yamlNode["albedoMap"].as<Guid>() };
  SetAlbedoMap(albedoMapGuid.IsValid()
                 ? gResourceManager.Load<Texture2D>(albedoMapGuid)
                 : GetAlbedoMap());

  auto const metallicMapGuid{ yamlNode["metallicMap"].as<Guid>() };
  SetMetallicMap(metallicMapGuid.IsValid()
                   ? gResourceManager.Load<Texture2D>(metallicMapGuid)
                   : GetMetallicMap());

  auto const roughnessMapGuid{ yamlNode["roughnessMap"].as<Guid>() };
  SetRoughnessMap(roughnessMapGuid.IsValid()
                    ? gResourceManager.Load<Texture2D>(roughnessMapGuid)
                    : GetRoughnessMap());

  auto const aoMapGuid{ yamlNode["aoMap"].as<Guid>() };
  SetAoMap(aoMapGuid.IsValid()
             ? gResourceManager.Load<Texture2D>(aoMapGuid)
             : GetAoMap());

  auto const normalMapGuid{ yamlNode["normalMap"].as<Guid>() };
  SetNormalMap(normalMapGuid.IsValid()
                 ? gResourceManager.Load<Texture2D>(normalMapGuid)
                 : GetNormalMap());
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

    if (Vector3 albedoColor{ GetAlbedoVector() }; ImGui::ColorEdit3("###matAlbedoColor", albedoColor.GetData())) {
      SetAlbedoVector(albedoColor);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Metallic");
    ImGui::TableNextColumn();

    if (f32 metallic{ GetMetallic() }; ImGui::SliderFloat("###matMetallic", &metallic, 0.0f, 1.0f)) {
      SetMetallic(metallic);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Roughness");
    ImGui::TableNextColumn();

    if (f32 roughness{ GetRoughness() }; ImGui::SliderFloat("###matRoughness", &roughness, 0.0f, 1.0f)) {
      SetRoughness(roughness);
      changed = true;
    }

    ImGui::TableNextColumn();
    ImGui::Text("Ambient Occlusion");
    ImGui::TableNextColumn();

    if (f32 ao{ GetAo() }; ImGui::SliderFloat("###matAo", &ao, 0.0f, 1.0f)) {
      SetAo(ao);
      changed = true;
    }

    auto drawTextureOption{
      [callCount = 0, &changed]<typename TexGetFn, typename TexSetFn>(std::string_view const label, TexGetFn&& texGetFn, TexSetFn&& texSetFn) mutable requires std::is_invocable_r_v<ObserverPtr<Texture2D>, TexGetFn> && std::is_invocable_r_v<void, TexSetFn, ObserverPtr<Texture2D>> {
        auto constexpr nullTexName{ "None" };
        static std::vector<ObserverPtr<Texture2D>> textures;
        static std::string texFilter;

        ImGui::TableNextColumn();
        ImGui::Text("%s", label.data());
        ImGui::TableNextColumn();

        auto const popupId{ std::format("SelectTexturePopUp{}", callCount) };

        auto const queryTextures{
          [] {
            FindObjectsOfType(textures);

            std::erase_if(textures, [](auto const tex) {
              return tex && !Contains(tex->GetName(), texFilter);
            });

            std::ranges::sort(textures, [](auto const lhs, auto const rhs) {
              return !lhs || (rhs && lhs->GetName() < rhs->GetName());
            });

            textures.insert(std::begin(textures), nullptr);
          }
        };

        if (ImGui::BeginPopup(popupId.data())) {
          if (ImGui::InputText("###SearchTextures", &texFilter)) {
            queryTextures();
          }

          for (auto const tex : textures) {
            if (ImGui::Selectable(std::format("{}##texoption{}", tex
                                                                   ? tex->GetName()
                                                                   : nullTexName, tex
                                                                                    ? tex->GetGuid().ToString()
                                                                                    : "0").c_str(), tex == texGetFn())) {
              texSetFn(tex);
              changed = true;

              break;
            }
          }

          ImGui::EndPopup();
        }

        if (ImGui::Button(std::format("Select##SelectTextureButton{}", callCount).data())) {
          texFilter.clear();
          queryTextures();
          ImGui::OpenPopup(popupId.data());
        }

        ImGui::SameLine();
        ImGui::Text("%s", texGetFn()
                            ? texGetFn()->GetName().data()
                            : nullTexName);

        callCount += 1;
      }
    };

    drawTextureOption("Albedo Map",
                      [this] {
                        return GetAlbedoMap();
                      },
                      [this](auto const tex) -> void {
                        SetAlbedoMap(tex);
                      });

    drawTextureOption("Metallic Map",
                      [this] {
                        return GetMetallicMap();
                      },
                      [this](auto const tex) -> void {
                        SetMetallicMap(tex);
                      });

    drawTextureOption("Roughness Map",
                      [this] {
                        return GetRoughnessMap();
                      },
                      [this](auto const tex) -> void {
                        SetRoughnessMap(tex);
                      });

    drawTextureOption("Ambient Occlusion Map",
                      [this] {
                        return GetAoMap();
                      },
                      [this](auto const tex) -> void {
                        SetAoMap(tex);
                      });

    drawTextureOption("Normal Map",
                      [this] {
                        return GetNormalMap();
                      },
                      [this](auto const tex) -> void {
                        SetNormalMap(tex);
                      });

    ImGui::EndTable();
  }
}
}
