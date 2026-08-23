#include "viewport.hpp"

#include "reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::NormalizedViewport>{"NormalizedViewport"}.
    constructor()(rttr::policy::ctor::as_object).property("left", &sorcery::NormalizedViewport::left).
    property("top", &sorcery::NormalizedViewport::top).property("right", &sorcery::NormalizedViewport::right).property(
      "bottom", &sorcery::NormalizedViewport::bottom);
}
