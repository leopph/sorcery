#include "SceneViewWindow.hpp"

#include <ImGuizmo.h>

#include "EditorCamera.hpp"
#include "Platform.hpp"
#include "Timing.hpp"


namespace sorcery::mage {
auto SceneViewWindow::Draw(Application& context) -> void {
  ImVec2 static constexpr sceneViewportMinSize{480, 270};

  ImGui::SetNextWindowSize(sceneViewportMinSize, ImGuiCond_FirstUseEver);
  ImGui::SetNextWindowSizeConstraints(sceneViewportMinSize, ImGui::GetMainViewport()->WorkSize);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

  if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_NoCollapse)) {
    ImGui::PopStyleVar();
    auto const contentRegionSize{ImGui::GetContentRegionAvail()};
    Extent2D const desiredRes{static_cast<u32>(contentRegionSize.x), static_cast<u32>(contentRegionSize.y)};

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

    static bool isMovingSceneCamera{false};

    auto const wasMovingSceneCamera{isMovingSceneCamera};
    isMovingSceneCamera = wasMovingSceneCamera
                            ? ImGui::IsMouseDown(ImGuiMouseButton_Right)
                            : ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Right);

    if (!wasMovingSceneCamera && isMovingSceneCamera) {
      gWindow.LockCursor(gWindow.GetCursorPosition());
      gWindow.SetCursorHiding(true);
    } else if (wasMovingSceneCamera && !isMovingSceneCamera) {
      gWindow.UnlockCursor();
      gWindow.SetCursorHiding(false);
    }

    if (isMovingSceneCamera) {
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

      float constexpr editorCamSpeed{5.0f};

      mEditorCam.position += mEditorCam.orientation.Rotate(posDelta) * editorCamSpeed * timing::GetFrameTime();

      auto const [mouseX, mouseY]{gWindow.GetMouseDelta()};
      auto constexpr sens{0.05f};

      mEditorCam.orientation = Quaternion{Vector3::Up(), static_cast<f32>(mouseX) * sens} * mEditorCam.orientation;
      mEditorCam.orientation *= Quaternion{Vector3::Right(), static_cast<f32>(mouseY) * sens};
    }

    if (auto const selectedObject{context.GetSelectedObject()}) {
      selectedObject->OnDrawGizmosSelected();
    }

    gRenderer.DrawCamera(mEditorCam, mRenderTarget.get());
    gRenderer.DrawGizmos(mRenderTarget.get());
    ImGui::Image(mRenderTarget->GetColorSrv(), contentRegionSize);

    auto const windowAspectRatio{ImGui::GetWindowWidth() / ImGui::GetWindowHeight()};
    auto const editorCamViewMat{mEditorCam.CalculateViewMatrix()};
    auto const editorCamProjMat{mEditorCam.CalculateProjectionMatrix(windowAspectRatio)};

    ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowWidth(), ImGui::GetWindowHeight());
    ImGuizmo::AllowAxisFlip(false);
    ImGuizmo::SetDrawlist();

    static bool showGrid{false};

    if (GetKeyDown(Key::G)) {
      showGrid = !showGrid;
    }

    if (showGrid) {
      ImGuizmo::DrawGrid(editorCamViewMat.GetData(), editorCamProjMat.GetData(), Matrix4::Identity().GetData(), mEditorCam.GetFarClipPlane());
    }

    if (auto const selectedEntity{dynamic_cast<Entity*>(context.GetSelectedObject())}; selectedEntity) {
      static auto op{ImGuizmo::OPERATION::TRANSLATE};

      if (!context.GetImGuiIo().WantTextInput && !isMovingSceneCamera) {
        if (GetKeyDown(Key::T)) {
          op = ImGuizmo::TRANSLATE;
        }
        if (GetKeyDown(Key::R)) {
          op = ImGuizmo::ROTATE;
        }
        if (GetKeyDown(Key::S)) {
          op = ImGuizmo::SCALE;
        }
        if (GetKeyDown(Key::F)) {
          mEditorCam.position = selectedEntity->GetTransform().GetWorldPosition() - Vector3::Forward() * 2;
          mEditorCam.orientation = selectedEntity->GetTransform().GetLocalRotation();
        }
      }

      if (Matrix4 modelMat{selectedEntity->GetTransform().GetModelMatrix()}; Manipulate(editorCamViewMat.GetData(), editorCamProjMat.GetData(), op, ImGuizmo::MODE::LOCAL, modelMat.GetData())) {
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
}
