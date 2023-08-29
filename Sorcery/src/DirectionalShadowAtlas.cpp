#include "DirectionalShadowAtlas.hpp"

#include "Bounds.hpp"
#include "Entity.hpp"
#include "Graphics.hpp"
#include "MemoryAllocation.hpp"

#include <array>
#include <format>


namespace sorcery {
DirectionalShadowAtlas::DirectionalShadowAtlas(ID3D11Device* const device, int const size):
  ShadowAtlas{device, size, 1},
  mCell{1} {}


auto DirectionalShadowAtlas::Update(std::span<LightComponent const* const> const allLights, Visibility const& visibility, Camera const& cam, ShadowCascadeBoundaries const& shadowCascadeBoundaries, float const aspectRatio, int const cascadeCount) -> void {
  std::pmr::vector<int> candidateLightIdxIndices{&GetTmpMemRes()};

  for (int i = 0; i < std::ssize(visibility.lightIndices); i++) {
    if (auto const& light{allLights[visibility.lightIndices[i]]}; light->IsCastingShadow() && light->GetType() == LightComponent::Type::Directional) {
      candidateLightIdxIndices.emplace_back(i);
    }
  }

  auto newCellSubdiv{1};

  while (newCellSubdiv * newCellSubdiv < std::ssize(candidateLightIdxIndices) * cascadeCount) {
    newCellSubdiv = NextPowerOfTwo(newCellSubdiv);
  }

  mCell.Resize(newCellSubdiv);

  for (int i = 0; i < mCell.GetElementCount(); i++) {
    mCell.GetSubcell(i).reset();
  }

  float const camNear{cam.GetNearClipPlane()};
  float const camFar{cam.GetFarClipPlane()};

  enum FrustumVertex : int {
    FrustumVertex_NearTopRight    = 0,
    FrustumVertex_NearTopLeft     = 1,
    FrustumVertex_NearBottomLeft  = 2,
    FrustumVertex_NearBottomRight = 3,
    FrustumVertex_FarTopRight     = 4,
    FrustumVertex_FarTopLeft      = 5,
    FrustumVertex_FarBottomLeft   = 6,
    FrustumVertex_FarBottomRight  = 7,
  };

  // Order of vertices is CCW from top right, near first
  auto const frustumVertsWS{
    [&cam, aspectRatio, camNear, camFar] {
      std::array<Vector3, 8> ret;

      Vector3 const nearWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camNear};
      Vector3 const farWorldForward{cam.GetPosition() + cam.GetForwardAxis() * camFar};

      switch (cam.GetType()) {
        case Camera::Type::Perspective: {
          float const tanHalfFov{std::tan(ToRadians(cam.GetHorizontalPerspectiveFov() / 2.0f))};
          float const nearExtentX{camNear * tanHalfFov};
          float const nearExtentY{nearExtentX / aspectRatio};
          float const farExtentX{camFar * tanHalfFov};
          float const farExtentY{farExtentX / aspectRatio};

          ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
          ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
          ret[FrustumVertex_NearBottomLeft] = nearWorldForward - cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
          ret[FrustumVertex_NearBottomRight] = nearWorldForward + cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
          ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
          ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
          ret[FrustumVertex_FarBottomLeft] = farWorldForward - cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
          ret[FrustumVertex_FarBottomRight] = farWorldForward + cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
          break;
        }
        case Camera::Type::Orthographic: {
          float const extentX{cam.GetHorizontalOrthographicSize() / 2.0f};
          float const extentY{extentX / aspectRatio};

          ret[FrustumVertex_NearTopRight] = nearWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[FrustumVertex_NearTopLeft] = nearWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[FrustumVertex_NearBottomLeft] = nearWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[FrustumVertex_NearBottomRight] = nearWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[FrustumVertex_FarTopRight] = farWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[FrustumVertex_FarTopLeft] = farWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[FrustumVertex_FarBottomLeft] = farWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[FrustumVertex_FarBottomRight] = farWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          break;
        }
      }

      return ret;
    }()
  };

  auto const frustumDepth{camFar - camNear};

  for (auto i = 0; i < std::ssize(candidateLightIdxIndices); i++) {
    auto const lightIdxIdx{candidateLightIdxIndices[i]};
    auto const& light{*allLights[visibility.lightIndices[lightIdxIdx]]};

    for (auto cascadeIdx = 0; cascadeIdx < cascadeCount; cascadeIdx++) {
      // cascade vertices in world space
      auto const cascadeVertsWS{
        [&frustumVertsWS, &shadowCascadeBoundaries, cascadeIdx, camNear, frustumDepth] {
          auto const [cascadeNear, cascadeFar]{shadowCascadeBoundaries[cascadeIdx]};

          float const cascadeNearNorm{(cascadeNear - camNear) / frustumDepth};
          float const cascadeFarNorm{(cascadeFar - camNear) / frustumDepth};

          std::array<Vector3, 8> ret;

          for (int j = 0; j < 4; j++) {
            Vector3 const& from{frustumVertsWS[j]};
            Vector3 const& to{frustumVertsWS[j + 4]};

            ret[j] = Lerp(from, to, cascadeNearNorm);
            ret[j + 4] = Lerp(from, to, cascadeFarNorm);
          }

          return ret;
        }()
      };


      Vector3 cascadeCenterWS{Vector3::Zero()};

      for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
        cascadeCenterWS += cascadeVertWS;
      }

      cascadeCenterWS /= 8.0f;

      float sphereRadius{0.0f};

      for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
        sphereRadius = std::max(sphereRadius, Distance(cascadeCenterWS, cascadeVertWS));
      }

      auto const shadowMapSize{GetSize() / GetSubdivisionSize()};
      auto const worldUnitsPerTexel{sphereRadius * 2.0f / static_cast<float>(shadowMapSize)};

      //sphereRadius = std::round(sphereRadius / worldUnitsPerTexel) * worldUnitsPerTexel;

      Matrix4 const shadowViewMtx{Matrix4::LookToLH(Vector3::Zero(), light.GetDirection(), Vector3::Up())};
      cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx};
      cascadeCenterWS /= worldUnitsPerTexel;
      cascadeCenterWS[0] = std::floor(cascadeCenterWS[0]);
      cascadeCenterWS[1] = std::floor(cascadeCenterWS[1]);
      cascadeCenterWS *= worldUnitsPerTexel;
      cascadeCenterWS = Vector3{Vector4{cascadeCenterWS, 1} * shadowViewMtx.Inverse()};

      auto shadowNearClipPlane{-sphereRadius - light.GetShadowExtension()};
      auto shadowFarClipPlane{sphereRadius};
      Graphics::AdjustClipPlanesForReversedDepth(shadowNearClipPlane, shadowFarClipPlane);
      Matrix4 const shadowProjMtx{Matrix4::OrthographicAsymZLH(-sphereRadius, sphereRadius, sphereRadius, -sphereRadius, shadowNearClipPlane, shadowFarClipPlane)};

      auto const shadowViewProjMtx{Matrix4::LookToLH(cascadeCenterWS, light.GetDirection(), Vector3::Up()) * shadowProjMtx};

      mCell.GetSubcell(i * mCell.GetSubdivisionSize() + cascadeIdx).emplace(shadowViewProjMtx, lightIdxIdx, cascadeIdx);
    }
  }
}


[[nodiscard]] auto DirectionalShadowAtlas::GetCell(int const idx) const -> Cell const& {
  ThrowIfIndexIsInvalid(idx);
  return mCell;
}
}
