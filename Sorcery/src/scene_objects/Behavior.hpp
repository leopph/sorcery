#pragma once

#include "Component.hpp"


namespace sorcery {
class Behavior : public virtual Component {
  RTTR_ENABLE(Component)

public:
  virtual void Update() {}
};
}
