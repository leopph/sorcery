#include "sorcery/rendering/dxgi_format.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <DirectXTex.h>


namespace sorcery::rendering {
auto GetFormatChannelCount(DXGI_FORMAT const format) -> std::uint32_t {
  return DirectX::IsCompressed(format)
           ? [&] {
             switch (DirectX::MakeTypeless(format)) {
               case DXGI_FORMAT_BC1_TYPELESS:
                 return 3;
               case DXGI_FORMAT_BC2_TYPELESS:
                 [[fallthrough]];
               case DXGI_FORMAT_BC3_TYPELESS:
                 return 4;
               case DXGI_FORMAT_BC4_TYPELESS:
                 return 1;
               case DXGI_FORMAT_BC5_TYPELESS:
                 return 2;
               case DXGI_FORMAT_BC6H_TYPELESS:
                 return 3;
               case DXGI_FORMAT_BC7_TYPELESS:
                 return 4;
               default:
                 return 0;
             }
           }()
           : static_cast<std::uint32_t>(DirectX::BitsPerPixel(format) / DirectX::BitsPerColor(format));
}
}
