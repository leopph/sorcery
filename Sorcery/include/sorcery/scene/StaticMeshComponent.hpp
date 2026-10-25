#pragma once

#include <sorcery/scene/MeshComponentBase.hpp>


namespace sorcery {
class StaticMeshComponent final : public MeshComponentBase {
  RTTR_ENABLE(MeshComponentBase)

public:
  [[nodiscard]] LEOPPHAPI auto Clone() -> std::unique_ptr<SceneObject> override;
  LEOPPHAPI auto OnAfterEnteringWorld(World& world) -> void override;
  LEOPPHAPI auto OnBeforeExitingWorld(World& world) -> void override;
};
}
