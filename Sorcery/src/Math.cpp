#include "Math.hpp"
#include "Reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Vector2>{ "Vector 2" }
    .constructor()(rttr::policy::ctor::as_object);

  rttr::registration::class_<sorcery::Vector3>{ "Vector 3" }
    .constructor()(rttr::policy::ctor::as_object);

  rttr::registration::class_<sorcery::Vector4>{ "Vector 4" }
    .constructor()(rttr::policy::ctor::as_object);

  /*rttr::registration::class_<sorcery::Matrix2>{ "Matrix 2" }
    .constructor()(rttr::policy::ctor::as_object);

  rttr::registration::class_<sorcery::Matrix3>{ "Matrix 3" }
    .constructor()(rttr::policy::ctor::as_object);

  rttr::registration::class_<sorcery::Matrix4>{ "Matrix 4" }
    .constructor()(rttr::policy::ctor::as_object) TODO*/
  ;
}
