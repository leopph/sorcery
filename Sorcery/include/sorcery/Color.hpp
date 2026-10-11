#pragma once

#include <cstdint>

#include <sorcery/Core.hpp>
#include <sorcery/Math.hpp>


namespace sorcery {
struct Color {
  std::uint8_t red;
  std::uint8_t green;
  std::uint8_t blue;
  std::uint8_t alpha;

  Color() = default;
  LEOPPHAPI Color(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha);
  LEOPPHAPI explicit Color(Vector4 const& vec);
  LEOPPHAPI explicit operator Vector4() const;

  LEOPPHAPI static auto Black() -> Color;
  LEOPPHAPI static auto Red() -> Color;
  LEOPPHAPI static auto Green() -> Color;
  LEOPPHAPI static auto Blue() -> Color;
  LEOPPHAPI static auto Cyan() -> Color;
  LEOPPHAPI static auto Magenta() -> Color;
  LEOPPHAPI static auto Yellow() -> Color;
  LEOPPHAPI static auto White() -> Color;
};
}
