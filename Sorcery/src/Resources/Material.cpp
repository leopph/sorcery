#include "Material.hpp"

#include "../Renderer.hpp"
#include "../Serialization.hpp"
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


auto Material::GetAlbedoMap() const noexcept -> ResourceHandle<Texture2D> {
  return mAlbedoMap;
}


auto Material::SetAlbedoMap(ResourceHandle<Texture2D> tex) noexcept -> void {
  mAlbedoMap = std::move(tex);
  mShaderMtl.sampleAlbedo = mAlbedoMap != nullres;
  UpdateGPUData();
}


auto Material::GetMetallicMap() const noexcept -> ResourceHandle<Texture2D> {
  return mMetallicMap;
}


auto Material::SetMetallicMap(ResourceHandle<Texture2D> tex) noexcept -> void {
  mMetallicMap = std::move(tex);
  mShaderMtl.sampleMetallic = mMetallicMap != nullres;
  UpdateGPUData();
}


auto Material::GetRoughnessMap() const noexcept -> ResourceHandle<Texture2D> {
  return mRoughnessMap;
}


auto Material::SetRoughnessMap(ResourceHandle<Texture2D> tex) noexcept -> void {
  mRoughnessMap = std::move(tex);
  mShaderMtl.sampleRoughness = mRoughnessMap != nullres;
  UpdateGPUData();
}


auto Material::GetAoMap() const noexcept -> ResourceHandle<Texture2D> {
  return mAoMap;
}


auto Material::SetAoMap(ResourceHandle<Texture2D> tex) noexcept -> void {
  mAoMap = std::move(tex);
  mShaderMtl.sampleAo = mAoMap != nullres;
  UpdateGPUData();
}


auto Material::GetNormalMap() const noexcept -> ResourceHandle<Texture2D> {
  return mNormalMap;
}


auto Material::SetNormalMap(ResourceHandle<Texture2D> tex) noexcept -> void {
  mNormalMap = std::move(tex);
  mShaderMtl.sampleNormal = mNormalMap != nullres;
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

  auto const albedoMap{ GetAlbedoMap().Get() };
  ret["albedoMap"] = albedoMap
                       ? albedoMap->GetGuid()
                       : Guid::Invalid();

  auto const metallicMap{ GetMetallicMap().Get() };
  ret["metallicMap"] = metallicMap
                         ? metallicMap->GetGuid()
                         : Guid::Invalid();

  auto const roughnessMap{ GetRoughnessMap().Get() };
  ret["roughnessMap"] = roughnessMap
                          ? roughnessMap->GetGuid()
                          : Guid::Invalid();

  auto const aoMap{ GetAoMap().Get() };
  ret["aoMap"] = aoMap
                   ? aoMap->GetGuid()
                   : Guid::Invalid();

  auto const normalMap{ GetNormalMap().Get() };
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
}
