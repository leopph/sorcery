#include "DirectionalShadowAtlas.hpp"

#include "Bounds.hpp"
#include "Graphics.hpp"

#include <array>


namespace sorcery {
DirectionalShadowAtlas::DirectionalShadowAtlas(ID3D11Device* const device, int const size):
  ShadowAtlas{ device, size, 1 },
  mCell{ 1 } {}


auto DirectionalShadowAtlas::Update(std::span<LightComponent const* const> const allLights, Visibility const& visibility, Camera const& cam, ShadowCascadeBoundaries const& shadowCascadeBoundaries, float const aspectRatio, int const cascadeCount, bool const useStableCascadeProjection) -> void {
  std::vector<int> static candidateLightIdxIndices;
  candidateLightIdxIndices.clear();

  for (int i = 0; i < std::ssize(visibility.lightIndices); i++) {
    if (auto const& light{ allLights[visibility.lightIndices[i]] }; light->IsCastingShadow() && light->GetType() == LightComponent::Type::Directional) {
      candidateLightIdxIndices.emplace_back(i);
    }
  }

  auto newCellSubdiv{ 1 };

  while (newCellSubdiv * newCellSubdiv < std::ssize(candidateLightIdxIndices) * cascadeCount) {
    newCellSubdiv = NextPowerOfTwo(newCellSubdiv);
  }

  mCell.Resize(newCellSubdiv);

  for (int i = 0; i < mCell.GetElementCount(); i++) {
    mCell.GetSubcell(i).reset();
  }

  float const camNear{ cam.GetNearClipPlane() };
  float const camFar{ cam.GetFarClipPlane() };

  // Order of vertices is CCW from top right, near first
  auto const frustumVertsWS{
    [&cam, aspectRatio, camNear, camFar] {
      std::array<Vector3, 8> ret;

      Vector3 const nearWorldForward{ cam.GetPosition() + cam.GetForwardAxis() * camNear };
      Vector3 const farWorldForward{ cam.GetPosition() + cam.GetForwardAxis() * camFar };

      switch (cam.GetType()) {
        case Camera::Type::Perspective: {
          float const tanHalfFov{ std::tan(ToRadians(cam.GetHorizontalPerspectiveFov() / 2.0f)) };
          float const nearExtentX{ camNear * tanHalfFov };
          float const nearExtentY{ nearExtentX / aspectRatio };
          float const farExtentX{ camFar * tanHalfFov };
          float const farExtentY{ farExtentX / aspectRatio };

          ret[0] = nearWorldForward + cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
          ret[1] = nearWorldForward - cam.GetRightAxis() * nearExtentX + cam.GetUpAxis() * nearExtentY;
          ret[2] = nearWorldForward - cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
          ret[3] = nearWorldForward + cam.GetRightAxis() * nearExtentX - cam.GetUpAxis() * nearExtentY;
          ret[4] = farWorldForward + cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
          ret[5] = farWorldForward - cam.GetRightAxis() * farExtentX + cam.GetUpAxis() * farExtentY;
          ret[6] = farWorldForward - cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
          ret[7] = farWorldForward + cam.GetRightAxis() * farExtentX - cam.GetUpAxis() * farExtentY;
          break;
        }
        case Camera::Type::Orthographic: {
          float const extentX{ cam.GetHorizontalOrthographicSize() / 2.0f };
          float const extentY{ extentX / aspectRatio };

          ret[0] = nearWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[1] = nearWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[2] = nearWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[3] = nearWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[4] = farWorldForward + cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[5] = farWorldForward - cam.GetRightAxis() * extentX + cam.GetUpAxis() * extentY;
          ret[6] = farWorldForward - cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          ret[7] = farWorldForward + cam.GetRightAxis() * extentX - cam.GetUpAxis() * extentY;
          break;
        }
      }

      return ret;
    }()
  };

  auto const frustumDepth{ camFar - camNear };

  for (auto i = 0; i < std::ssize(candidateLightIdxIndices); i++) {
    auto const lightIdxIdx{ candidateLightIdxIndices[i] };
    auto const& light{ *allLights[visibility.lightIndices[lightIdxIdx]] };

    for (auto cascadeIdx = 0; cascadeIdx < cascadeCount; cascadeIdx++) {
      // cascade vertices in world space
      auto const cascadeVertsWS{
        [&frustumVertsWS, &shadowCascadeBoundaries, cascadeIdx, camNear, frustumDepth] {
          auto const [cascadeNear, cascadeFar]{ shadowCascadeBoundaries[cascadeIdx] };

          float const cascadeNearNorm{ (cascadeNear - camNear) / frustumDepth };
          float const cascadeFarNorm{ (cascadeFar - camNear) / frustumDepth };

          std::array<Vector3, 8> ret;

          for (int j = 0; j < 4; j++) {
            Vector3 const& from{ frustumVertsWS[j] };
            Vector3 const& to{ frustumVertsWS[j + 4] };

            ret[j] = Lerp(from, to, cascadeNearNorm);
            ret[j + 4] = Lerp(from, to, cascadeFarNorm);
          }

          return ret;
        }()
      };

      auto const calculateTightViewProj{
        [&cascadeVertsWS, &light] {
          Matrix4 const shadowViewMtx{ Matrix4::LookToLH(Vector3::Zero(), light.GetDirection(), Vector3::Up()) };

          // cascade vertices in shadow space
          auto const cascadeVertsSP{
            [&cascadeVertsWS, &shadowViewMtx] {
              std::array<Vector3, 8> ret;

              for (int j = 0; j < 8; j++) {
                ret[j] = Vector3{ Vector4{ cascadeVertsWS[j], 1 } * shadowViewMtx };
              }

              return ret;
            }()
          };

          auto const [aabbMin, aabbMax]{ AABB::FromVertices(cascadeVertsSP) };
          auto shadowNearClipPlane{ aabbMin[2] - light.GetShadowExtension() };
          auto shadowFarClipPlane{ aabbMax[2] };
          Graphics::AdjustClipPlanesForReversedDepth(shadowNearClipPlane, shadowFarClipPlane);
          Matrix4 const shadowProjMtx{ Matrix4::OrthographicAsymZLH(aabbMin[0], aabbMax[0], aabbMax[1], aabbMin[1], shadowNearClipPlane, shadowFarClipPlane) };

          return shadowViewMtx * shadowProjMtx;
        }
      };

      auto const calculateStableViewProj{
        [&cascadeVertsWS, &light, this] {
          Vector3 cascadeCenterWS{ Vector3::Zero() };

          for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
            cascadeCenterWS += cascadeVertWS;
          }

          cascadeCenterWS /= 8.0f;

          float sphereRadius{ 0.0f };

          for (Vector3 const& cascadeVertWS : cascadeVertsWS) {
            sphereRadius = std::max(sphereRadius, Distance(cascadeCenterWS, cascadeVertWS));
          }

          float const shadowMapSize{ static_cast<float>(static_cast<int>(GetSize() / GetSubdivisionSize())) };
          float const texelsPerUnit{ shadowMapSize / (sphereRadius * 2.0f) };

          Matrix4 const lookAtMtx{ Matrix4::LookToLH(Vector3::Zero(), light.GetDirection(), Vector3::Up()) };
          Matrix4 const scaleMtx{ Matrix4::Scale(Vector3{ texelsPerUnit }) };
          Matrix4 const baseViewMtx{ scaleMtx * lookAtMtx };
          Matrix4 const baseViewInvMtx{ baseViewMtx.Inverse() };

          Vector3 correctedCascadeCenter{ Vector4{ cascadeCenterWS, 1 } * baseViewMtx };

          for (int j = 0; j < 2; j++) {
            correctedCascadeCenter[j] = std::floor(correctedCascadeCenter[j]);
          }

          correctedCascadeCenter = Vector3{ Vector4{ correctedCascadeCenter, 1 } * baseViewInvMtx };

          Matrix4 const shadowViewMtx{ Matrix4::LookToLH(correctedCascadeCenter, light.GetDirection(), Vector3::Up()) };
          auto shadowNearClipPlane{ -sphereRadius - light.GetShadowExtension() };
          auto shadowFarClipPlane{ sphereRadius };
          Graphics::AdjustClipPlanesForReversedDepth(shadowNearClipPlane, shadowFarClipPlane);
          Matrix4 const shadowProjMtx{ Matrix4::OrthographicAsymZLH(-sphereRadius, sphereRadius, sphereRadius, -sphereRadius, shadowNearClipPlane, shadowFarClipPlane) };

          return shadowViewMtx * shadowProjMtx;
        }
      };

      auto const shadowViewProjMtx{ useStableCascadeProjection ? calculateStableViewProj() : calculateTightViewProj() };

      mCell.GetSubcell(i * mCell.GetSubdivisionSize() + cascadeIdx).emplace(shadowViewProjMtx, lightIdxIdx, cascadeIdx);
    }
  }
}


[[nodiscard]] auto DirectionalShadowAtlas::GetCell(int const idx) const -> Cell const& {
  ThrowIfIndexIsInvalid(idx);
  return mCell;
}
}
