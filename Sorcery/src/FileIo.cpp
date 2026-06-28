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
  std::vector<unsigned char> temp;

  if (!ReadFileBinary(src, temp)) {
    return false;
  }

  out.resize(temp.size());
  std::ranges::transform(temp, out.begin(), [](unsigned char const c) { return static_cast<std::byte>(c); });
  return true;
}
}
