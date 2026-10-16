#include "sorcery/Math.hpp"

#include "sorcery/reflection.hpp"

RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Vector2>{"Vector2"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Vector2::mData);

  rttr::registration::class_<sorcery::Vector3>{"Vector3"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Vector3::mData);

  rttr::registration::class_<sorcery::Vector4>{"Vector4"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Vector4::mData);

  rttr::registration::class_<sorcery::Matrix2>{"Matrix2"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Matrix2::mData);

  rttr::registration::class_<sorcery::Matrix3>{"Matrix3"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Matrix3::mData);

  rttr::registration::class_<sorcery::Matrix4>{"Matrix4"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("data", &sorcery::Matrix4::mData);

  rttr::registration::class_<sorcery::Quaternion>{"Quaternion"}
    .constructor()(rttr::policy::ctor::as_object)
    .property("x", &sorcery::Quaternion::x)
    .property("y", &sorcery::Quaternion::y)
    .property("z", &sorcery::Quaternion::z)
    .property("w", &sorcery::Quaternion::w);
}
