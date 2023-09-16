#include "SceneViewWindow.hpp"

#include "StandaloneCamera.hpp"
#include "Platform.hpp"
#include "Timing.hpp"
#include "Window.hpp"


namespace sorcery::mage {
auto SceneViewWindow::Draw(Application& context) -> void {
  ImGui::SetNextWindowSizeConstraints(ImVec2{480, 270}, ImVec2{std::numeric_limits<float>::max(), std::numeric_limits<float>::max()});
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_MenuBar)) {
    ImGui::PopStyleVar();


    if (ImGui::BeginMenuBar()) {
      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoOpCombo", GIZMO_OP_OPTIONS[mGizmoOpIdx].label)) {
        for (auto i{0}; i < std::ssize(GIZMO_OP_OPTIONS); i++) {
          if (ImGui::Selectable(GIZMO_OP_OPTIONS[i].label, mGizmoOpIdx == i)) {
            mGizmoOpIdx = i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::SetNextItemWidth(100);

      if (ImGui::BeginCombo("##GizmoModeCombo", GIZMO_MODE_OPTIONS[mGizmoModeIdx].label)) {
        for (auto i{0}; i < std::ssize(GIZMO_MODE_OPTIONS); i++) {
          if (ImGui::Selectable(GIZMO_MODE_OPTIONS[i].label, mGizmoModeIdx == i)) {
            mGizmoModeIdx = i;
          }
        }
        ImGui::EndCombo();
      }

      ImGui::EndMenuBar();
    }

    auto const contentRegionSize{ImGui::GetContentRegionAvail()};
    Extent2D const desiredRes{static_cast<std::uint32_t>(contentRegionSize.x), static_cast<std::uint32_t>(contentRegionSize.y)};

    if (!mRenderTarget || mRenderTarget->GetDesc().width != desiredRes.width || mRenderTarget->GetDesc().height != desiredRes.height) {
      mRenderTarget = std::make_unique<RenderTarget>(RenderTarget::Desc{
        .width = desiredRes.width,
        .height = desiredRes.height,
        .colorFormat = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
        .depthBufferBitCount = 0,
        .stencilBufferBitCount = 0,
        .debugName = "Game View RenderTarget"
      });
    }

    auto const wasCamMoving{mCamMoving};
    mCamMoving = wasCamMoving ? ImGui::IsMouseDown(ImGuiMouseButton_Right) : ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (!wasCamMoving && mCamMoving) {
      gWindow.SetCursorLock(GetCursorPosition());
      gWindow.SetCursorHiding(true);
    } else if (wasCamMoving && !mCamMoving) {
      gWindow.SetCursorLock(std::nullopt);
      gWindow.SetCursorHiding(false);
    }

    if (mCamMoving) {
      ImGui::SetWindowFocus();

      Vector3 posDelta{0, 0, 0};
      if (GetKey(Key::W) || GetKey(Key::UpArrow)) {
        posDelta += Vector3::Forward();
      }
      if (GetKey(Key::A) || GetKey(Key::LeftArrow)) {
        posDelta += Vector3::Left();
      }
      if (GetKey(Key::D) || GetKey(Key::RightArrow)) {
        posDelta += Vector3::Right();
      }
      if (GetKey(Key::S) || GetKey(Key::DownArrow)) {
        posDelta += Vector3::Backward();
      }

      Normalize(posDelta);

      if (GetKey(Key::Shift)) {
        posDelta *= 2;
      }

      mCam.position += mCam.orientation.Rotate(posDelta) * mCam.speed * timing::GetFrameTime();

      auto const [mouseX, mouseY]{GetMouseDelta()};
      auto constexpr sens{0.05f};

      mCam.orientation = Quaternion{Vector3::Up(), static_cast<f32>(mouseX) * sens} * mCam.orientation;
      mCam.orientation *= Quaternion{Vector3::Right(), static_cast<f32>(mouseY) * sens};
    }

    if (auto const selectedObject{context.GetSelectedObject()}) {
      selectedObject->OnDrawGizmosSelected();
    }

    gRenderer.DrawCamera(mCam, mRenderTarget.get());
    gRenderer.DrawGizmos(mRenderTarget.get());
    ImGui::Image(mRenderTarget->GetColorSrv(), contentRegionSize);

    auto const aspect{ImGui::GetWindowWidth() / ImGui::GetWindowHeight()};
    auto const camViewMtx{mCam.CalculateViewMatrix()};
    auto const camProjMtx{mCam.CalculateProjectionMatrix(aspect)};

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();

    if (ImGui::IsWindowFocused() && GetKeyDown(Key::G)) {
      mShowGrid = !mShowGrid;
    }

    if (mShowGrid) {
      ImGuizmo::DrawGrid(camViewMtx.GetData(), camProjMtx.GetData(), Matrix4::Identity().GetData(),
        mCam.GetFarClipPlane());
    }

    if (mFocusTarget) {
      mFocusTarget->t += timing::GetFrameTime() * 5.0f;
      mFocusTarget->t = std::min(mFocusTarget->t, 1.0f);
      mCam.position = Lerp(mFocusTarget->source, mFocusTarget->target, mFocusTarget->t);

      if (mFocusTarget->t >= 1.0f) {
        mFocusTarget.reset();
      }
    }

    if (!context.GetImGuiIo().WantTextInput && !mCamMoving) {
      if (GetKeyDown(Key::Q)) {
        mGizmoOpIdx = 0;
      }
      if (GetKeyDown(Key::W)) {
        mGizmoOpIdx = 1;
      }
      if (GetKeyDown(Key::E)) {
        mGizmoOpIdx = 2;
      }
      if (GetKeyDown(Key::R)) {
        mGizmoModeIdx = 1 - mGizmoModeIdx;
      }
    }

    if (auto const selectedEntity{dynamic_cast<Entity*>(context.GetSelectedObject())}; selectedEntity) {
      if (!context.GetImGuiIo().WantTextInput && !mCamMoving && GetKeyDown(Key::F)) {
        mFocusTarget.emplace(mCam.GetPosition(), selectedEntity->GetTransform().GetWorldPosition() - mCam.GetForwardAxis() * 2, 0.0f);
      }

      // TODO this breaks if the transform is a child of a rotated transform
      if (Matrix4 modelMat{selectedEntity->GetTransform().GetLocalToWorldMatrix()}; Manipulate(camViewMtx.GetData(), camProjMtx.GetData(), GIZMO_OP_OPTIONS[mGizmoOpIdx].op, GIZMO_MODE_OPTIONS[mGizmoModeIdx].mode, modelMat.GetData())) {
        Vector3 pos, euler, scale;
        ImGuizmo::DecomposeMatrixToComponents(modelMat.GetData(), pos.GetData(), euler.GetData(), scale.GetData());
        selectedEntity->GetTransform().SetWorldPosition(pos);
        selectedEntity->GetTransform().SetWorldRotation(Quaternion::FromEulerAngles(euler));
        selectedEntity->GetTransform().SetWorldScale(scale);
      }
    }
  } else {
    ImGui::PopStyleVar();
  }
  ImGui::End();
}


auto SceneViewWindow::GetCamera() noexcept -> StandaloneCamera& {
  return mCam;
}
}
