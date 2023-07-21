#include "Material.hpp"

#include "../Renderer.hpp"
#include "../Serialization.hpp"
#include "../Systems.hpp"
#undef FindResource
#include "../ResourceManager.hpp"

#include <cstdint>
#include <stdexcept>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Material>{ "Material" };
}


namespace sorcery {
Object::Type const Material::SerializationType{ Type::Material };


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


Material::Material(Vector3 const& albedoVector, float const metallic, float const roughness, float const ao, std::weak_ptr<Texture2D> albedoMap, std::weak_ptr<Texture2D> metallicMap, std::weak_ptr<Texture2D> roughnessMap, std::weak_ptr<Texture2D> aoMap, std::weak_ptr<Texture2D> normalMap) :
  mShaderMtl{
    .albedo = albedoVector,
    .metallic = metallic,
    .roughness = roughness,
    .ao = ao,
    .sampleAlbedo = albedoMap.lock() != nullptr,
    .sampleMetallic = metallicMap.lock() != nullptr,
    .sampleRoughness = roughnessMap.lock() != nullptr,
    .sampleAo = aoMap.lock() != nullptr,
    .sampleNormal = normalMap.lock() != nullptr
  },
  mAlbedoMap{ std::move(albedoMap) },
  mMetallicMap{ std::move(metallicMap) },
  mRoughnessMap{ std::move(roughnessMap) },
  mAoMap{ std::move(aoMap) },
  mNormalMap{ std::move(normalMap) } {
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


auto Material::GetAlbedoMap() const noexcept -> std::weak_ptr<Texture2D> {
  return mAlbedoMap;
}


auto Material::SetAlbedoMap(std::weak_ptr<Texture2D> tex) noexcept -> void {
  mAlbedoMap = std::move(tex);
  mShaderMtl.sampleAlbedo = mAlbedoMap.lock() != nullptr;
  UpdateGPUData();
}


auto Material::GetMetallicMap() const noexcept -> std::weak_ptr<Texture2D> {
  return mMetallicMap;
}


auto Material::SetMetallicMap(std::weak_ptr<Texture2D> tex) noexcept -> void {
  mMetallicMap = std::move(tex);
  mShaderMtl.sampleMetallic = mMetallicMap.lock() != nullptr;
  UpdateGPUData();
}


auto Material::GetRoughnessMap() const noexcept -> std::weak_ptr<Texture2D> {
  return mRoughnessMap;
}


auto Material::SetRoughnessMap(std::weak_ptr<Texture2D> tex) noexcept -> void {
  mRoughnessMap = std::move(tex);
  mShaderMtl.sampleRoughness = mRoughnessMap.lock() != nullptr;
  UpdateGPUData();
}


auto Material::GetAoMap() const noexcept -> std::weak_ptr<Texture2D> {
  return mAoMap;
}


auto Material::SetAoMap(std::weak_ptr<Texture2D> tex) noexcept -> void {
  mAoMap = std::move(tex);
  mShaderMtl.sampleAo = mAoMap.lock() != nullptr;
  UpdateGPUData();
}


auto Material::GetNormalMap() const noexcept -> std::weak_ptr<Texture2D> {
  return mNormalMap;
}


auto Material::SetNormalMap(std::weak_ptr<Texture2D> tex) noexcept -> void {
  mNormalMap = std::move(tex);
  mShaderMtl.sampleNormal = mNormalMap.lock() != nullptr;
  UpdateGPUData();
}


auto Material::GetBuffer() const noexcept -> ObserverPtr<ID3D11Buffer> {
  return mCB.Get();
}


auto Material::GetSerializationType() const -> Type {
  return SerializationType;
}


auto Material::Serialize() const noexcept -> YAML::Node {
  YAML::Node ret;

  ret["albedo"] = GetAlbedoVector();
  ret["metallic"] = GetMetallic();
  ret["roughness"] = GetRoughness();
  ret["ao"] = GetAo();

  auto const albedoMap{ GetAlbedoMap().lock() };
  ret["albedoMap"] = static_cast<std::string>(albedoMap
                                                ? albedoMap->GetGuid()
                                                : Guid::Invalid());

  auto const metallicMap{ GetMetallicMap().lock() };
  ret["metallicMap"] = static_cast<std::string>(metallicMap
                                                  ? metallicMap->GetGuid()
                                                  : Guid::Invalid());

  auto const roughnessMap{ GetRoughnessMap().lock() };
  ret["roughnessMap"] = static_cast<std::string>(roughnessMap
                                                   ? roughnessMap->GetGuid()
                                                   : Guid::Invalid());

  auto const aoMap{ GetAoMap().lock() };
  ret["aoMap"] = static_cast<std::string>(aoMap
                                            ? aoMap->GetGuid()
                                            : Guid::Invalid());

  auto const normalMap{ GetNormalMap().lock() };
  ret["normalMap"] = static_cast<std::string>(normalMap
                                                ? normalMap->GetGuid()
                                                : Guid::Invalid());

  return ret;
}


auto Material::Deserialize(YAML::Node const& yamlNode) noexcept -> void {
  SetAlbedoVector(yamlNode["albedo"].as<Vector3>(GetAlbedoVector()));
  SetMetallic(yamlNode["metallic"].as<float>(GetMetallic()));
  SetRoughness(yamlNode["roughness"].as<float>(GetRoughness()));
  SetAo(yamlNode["ao"].as<float>(GetAo()));

  auto const albedoMapGuid{ Guid::Parse(yamlNode["albedoMap"].as<std::string>()) };
  SetAlbedoMap(albedoMapGuid.IsValid()
                 ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(albedoMapGuid).lock())
                 : GetAlbedoMap());

  auto const metallicMapGuid{ Guid::Parse(yamlNode["metallicMap"].as<std::string>()) };
  SetMetallicMap(metallicMapGuid.IsValid()
                   ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(metallicMapGuid).lock())
                   : GetMetallicMap());

  auto const roughnessMapGuid{ Guid::Parse(yamlNode["roughnessMap"].as<std::string>()) };
  SetRoughnessMap(roughnessMapGuid.IsValid()
                    ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(roughnessMapGuid).lock())
                    : GetRoughnessMap());

  auto const aoMapGuid{ Guid::Parse(yamlNode["aoMap"].as<std::string>()) };
  SetAoMap(aoMapGuid.IsValid()
             ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(aoMapGuid).lock())
             : GetAoMap());

  auto const normalMapGuid{ Guid::Parse(yamlNode["normalMap"].as<std::string>()) };
  SetNormalMap(normalMapGuid.IsValid()
                 ? std::dynamic_pointer_cast<Texture2D>(gResourceManager.FindResource(normalMapGuid).lock())
                 : GetNormalMap());
}
}
