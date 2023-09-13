#include "PunctualShadowAtlas.hpp"

#include "../Bounds.hpp"
#include "Graphics.hpp"
#include "../SceneObjects/TransformComponent.hpp"
#include "../SceneObjects/Entity.hpp"
#include "../MemoryAllocation.hpp"


namespace sorcery {
PunctualShadowAtlas::PunctualShadowAtlas(ID3D11Device* const device, int const size):
  ShadowAtlas{device, size, 2},
  mCells{Cell{1}, Cell{2}, Cell{4}, Cell{8}} {}


auto PunctualShadowAtlas::Update(std::span<LightComponent const* const> const allLights, Visibility const& visibility,
                                 Camera const& cam, Matrix4 const& camViewProjMtx, float const shadowDistance) -> void {
  struct LightCascadeIndex {
    int lightIdxIdx;
    int shadowIdx;
  };

  auto& tmpMemRes{GetTmpMemRes()};
  std::array lightIndexIndicesInCell{
    std::pmr::vector<LightCascadeIndex>{&tmpMemRes},
    std::pmr::vector<LightCascadeIndex>{&tmpMemRes},
    std::pmr::vector<LightCascadeIndex>{&tmpMemRes},
    std::pmr::vector<LightCascadeIndex>{&tmpMemRes}
  };

  auto const& camPos{cam.GetPosition()};

  auto const determineScreenCoverage{
    [&camPos, &camViewProjMtx](std::span<Vector3 const> const vertices) -> std::optional<int> {
      std::optional<int> cellIdx;

      if (auto const [worldMin, worldMax]{AABB::FromVertices(vertices)};
        worldMin[0] <= camPos[0] && worldMin[1] <= camPos[1] && worldMin[2] <= camPos[2] &&
        worldMax[0] >= camPos[0] && worldMax[1] >= camPos[1] && worldMax[2] >= camPos[2]) {
        cellIdx = 0;
      } else {
        Vector2 const bottomLeft{-1, -1};
        Vector2 const topRight{1, 1};

        Vector2 min{std::numeric_limits<float>::max()};
        Vector2 max{std::numeric_limits<float>::lowest()};

        for (auto& vertex : vertices) {
          Vector4 vertex4{vertex, 1};
          vertex4 *= camViewProjMtx;
          auto const projected{Vector2{vertex4} / vertex4[3]};
          min = Clamp(Min(min, projected), bottomLeft, topRight);
          max = Clamp(Max(max, projected), bottomLeft, topRight);
        }

        auto const width{max[0] - min[0]};
        auto const height{max[1] - min[1]};

        auto const area{width * height};
        auto const coverage{area / 4};

        if (coverage >= 1) {
          cellIdx = 0;
        } else if (coverage >= 0.25f) {
          cellIdx = 1;
        } else if (coverage >= 0.0625f) {
          cellIdx = 2;
        } else if (coverage >= 0.015625f) {
          cellIdx = 3;
        }
      }

      return cellIdx;
    }
  };

  for (int i = 0; i < static_cast<int>(visibility.lightIndices.size()); i++) {
    if (auto const light{allLights[visibility.lightIndices[i]]};
      light->IsCastingShadow() && (light->GetType() == LightComponent::Type::Spot || light->GetType() ==
                                   LightComponent::Type::Point)) {
      Vector3 const& lightPos{light->GetEntity().GetTransform().GetWorldPosition()};
      float const lightRange{light->GetRange()};

      // Skip the light if its bounding sphere is farther than the shadow distance
      if (Vector3 const camToLightDir{Normalize(lightPos - camPos)}; Distance(lightPos - camToLightDir * lightRange,
                                                                       cam.GetPosition()) > shadowDistance) {
        continue;
      }

      if (light->GetType() == LightComponent::Type::Spot) {
        auto lightVertices{CalculateSpotLightLocalVertices(*light)};

        for (auto const modelMtxNoScale{light->GetEntity().GetTransform().CalculateLocalToWorldMatrixWithoutScale()};
             auto& vertex : lightVertices) {
          vertex = Vector3{Vector4{vertex, 1} * modelMtxNoScale};
        }

        if (auto const cellIdx{determineScreenCoverage(lightVertices)}) {
          lightIndexIndicesInCell[*cellIdx].emplace_back(i, 0);
        }
      } else if (light->GetType() == LightComponent::Type::Point) {
        for (auto j = 0; j < 6; j++) {
          std::array static const faceBoundsRotations{
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(90)), // +X
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(-90)), // -X
            Quaternion::FromAxisAngle(Vector3::Right(), ToRadians(-90)), // +Y
            Quaternion::FromAxisAngle(Vector3::Right(), ToRadians(90)), // -Y
            Quaternion{}, // +Z
            Quaternion::FromAxisAngle(Vector3::Up(), ToRadians(180)) // -Z
          };

          std::array const shadowFrustumVertices{
            faceBoundsRotations[i].Rotate(Vector3{lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{-lightRange, lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{-lightRange, -lightRange, lightRange}) + lightPos,
            faceBoundsRotations[i].Rotate(Vector3{lightRange, -lightRange, lightRange}) + lightPos,
            lightPos,
          };

          if (auto const cellIdx{determineScreenCoverage(shadowFrustumVertices)}) {
            lightIndexIndicesInCell[*cellIdx].emplace_back(i, j);
          }
        }
      }
    }
  }

  for (int i = 0; i < 4; i++) {
    std::ranges::sort(lightIndexIndicesInCell[i],
      [&visibility, &camPos, &allLights](LightCascadeIndex const lhs, LightCascadeIndex const rhs) {
        auto const leftLight{allLights[visibility.lightIndices[lhs.lightIdxIdx]]};
        auto const rightLight{allLights[visibility.lightIndices[rhs.lightIdxIdx]]};

        auto const leftLightPos{leftLight->GetEntity().GetTransform().GetWorldPosition()};
        auto const rightLightPos{rightLight->GetEntity().GetTransform().GetWorldPosition()};

        auto const leftDist{Distance(leftLightPos, camPos)};
        auto const rightDist{Distance(camPos, rightLightPos)};

        return leftDist > rightDist;
      });

    for (int j = 0; j < mCells[i].GetElementCount(); j++) {
      auto& subcell{mCells[i].GetSubcell(j)};
      subcell.reset();

      if (lightIndexIndicesInCell[i].empty()) {
        continue;
      }

      auto const [lightIdxIdx, shadowIdx]{lightIndexIndicesInCell[i].back()};
      auto const light{allLights[visibility.lightIndices[lightIdxIdx]]};
      lightIndexIndicesInCell[i].pop_back();

      if (light->GetType() == LightComponent::Type::Spot) {
        auto const shadowViewMtx{
          Matrix4::LookTo(light->GetEntity().GetTransform().GetWorldPosition(),
            light->GetEntity().GetTransform().GetForwardAxis(), Vector3::Up())
        };
        auto const shadowProjMtx{
          Matrix4::PerspectiveFov(ToRadians(light->GetOuterAngle()), 1.f, light->GetRange(),
            light->GetShadowNearPlane())
        };

        subcell.emplace(shadowViewMtx * shadowProjMtx, lightIdxIdx, shadowIdx);
      } else if (light->GetType() == LightComponent::Type::Point) {
        auto const lightPos{light->GetEntity().GetTransform().GetWorldPosition()};

        std::array const faceViewMatrices{
          Matrix4::LookTo(lightPos, Vector3::Right(), Vector3::Up()), // +X
          Matrix4::LookTo(lightPos, Vector3::Left(), Vector3::Up()), // -X
          Matrix4::LookTo(lightPos, Vector3::Up(), Vector3::Backward()), // +Y
          Matrix4::LookTo(lightPos, Vector3::Down(), Vector3::Forward()), // -Y
          Matrix4::LookTo(lightPos, Vector3::Forward(), Vector3::Up()), // +Z
          Matrix4::LookTo(lightPos, Vector3::Backward(), Vector3::Up()), // -Z
        };

        auto const shadowViewMtx{faceViewMatrices[shadowIdx]};
        auto const shadowProjMtx{
          Graphics::GetProjectionMatrixForRendering(Matrix4::PerspectiveFov(ToRadians(90), 1,
            light->GetShadowNearPlane(), light->GetRange()))
        };

        subcell.emplace(shadowViewMtx * shadowProjMtx, lightIdxIdx, shadowIdx);
      }
    }

    if (i + 1 < 4) {
      std::ranges::copy(lightIndexIndicesInCell[i], std::back_inserter(lightIndexIndicesInCell[i + 1]));
    }
  }
}


auto PunctualShadowAtlas::GetCell(int const idx) const -> Cell const& {
  ThrowIfIndexIsInvalid(idx);
  return mCells[idx];
}
}
