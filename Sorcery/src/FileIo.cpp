#include "FileIo.hpp"

#include <fast_io.h>


namespace sorcery {
auto ReadFileBinary(std::filesystem::path const& src, std::vector<unsigned char>& out) -> bool {
  fast_io::native_file_loader const loader{src};

  if (loader.empty()) {
    return false;
  }

  out.assign(loader.begin(), loader.end());
  return true;
}
}
