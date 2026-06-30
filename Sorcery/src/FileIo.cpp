#include "FileIo.hpp"

#include <algorithm>

#include <fast_io.h>


namespace sorcery {
auto ReadFileBinary(
  std::filesystem::path const& src,
  std::vector<unsigned char>& out
) -> bool {
  fast_io::native_file_loader loader{src};

  if (loader.empty()) {
    return false;
  }

  out.resize(loader.size());
  out.assign(loader.begin(), loader.end());
  return true;
}


auto ReadFileBinary(
  std::filesystem::path const& src,
  std::vector<std::byte>& out
) -> bool {
  fast_io::native_file_loader loader{src};

  if (loader.empty()) {
    return false;
  }

  out.resize(loader.size());
  std::ranges::transform(loader, out.begin(), [](char const chr) {
    return static_cast<std::byte>(chr);
  });

  return true;
}
}
