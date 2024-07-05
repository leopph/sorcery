#pragma once

#include "MeshComponentBase.hpp"


namespace sorcery {
class StaticMeshComponent final : public MeshComponentBase {
  RTTR_ENABLE(MeshComponentBase)

public:
  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringScene(Scene const& scene) -> void override;
  LEOPPHAPI auto OnBeforeExitingScene(Scene const& scene) -> void override;
};
}
