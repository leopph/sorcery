#include "Color.hpp"
#include "Util.hpp"
#include "Reflection.hpp"

#include <algorithm>


RTTR_REGISTRATION {
  rttr::registration::class_<sorcery::Color>{ "Color" }
    .constructor()(rttr::policy::ctor::as_object)
    .constructor<sorcery::u8, sorcery::u8, sorcery::u8, sorcery::u8>()(rttr::policy::ctor::as_object)
    .constructor<sorcery::Vector4 const&>()(rttr::policy::ctor::as_object)
    .property("red", &sorcery::Color::red)
    .property("green", &sorcery::Color::green)
    .property("blue", &sorcery::Color::blue)
    .property("alpha", &sorcery::Color::alpha);
}


namespace sorcery {
Color::Color(u8 const red, u8 const green, u8 const blue, u8 const alpha) :
  red{ red },
  green{ green },
  blue{ blue },
  alpha{ alpha } {}


Color::Color(Vector4 const& vec) :
  red{ static_cast<u8>(std::clamp(vec[0], 0.f, 1.f) * 255) },
  green{ static_cast<u8>(std::clamp(vec[1], 0.f, 1.f) * 255) },
  blue{ static_cast<u8>(std::clamp(vec[2], 0.f, 1.f) * 255) },
  alpha{ static_cast<u8>(std::clamp(vec[3], 0.f, 1.f) * 255) } {}


Color::operator Vector4() const {
  return Vector4
  {
    static_cast<f32>(red) / 255.f,
    static_cast<f32>(green) / 255.f,
    static_cast<f32>(blue) / 255.f,
    static_cast<f32>(alpha) / 255.f
  };
}


auto Color::Black() -> Color {
  return Color{ 0, 0, 0, 255 };
}


auto Color::Red() -> Color {
  return Color{ 255, 0, 0, 255 };
}


auto Color::Green() -> Color {
  return Color{ 0, 255, 0, 255 };
}


auto Color::Blue() -> Color {
  return Color{ 0, 0, 255, 255 };
}


auto Color::Cyan() -> Color {
  return Color{ 0, 255, 255, 255 };
}


auto Color::Magenta() -> Color {
  return Color{ 255, 0, 255, 255 };
}


auto Color::Yellow() -> Color {
  return Color{ 255, 255, 0, 255 };
}


auto Color::White() -> Color {
  return Color{ 255, 255, 255, 255 };
}
}
