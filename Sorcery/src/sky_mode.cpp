#include "sorcery/sky_mode.hpp"

#include "sorcery/reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::enumeration<sorcery::SkyMode>("SkyMode")(
    rttr::value("Color", sorcery::SkyMode::kColor),
    rttr::value("Skybox", sorcery::SkyMode::kSkybox)
  );
}
