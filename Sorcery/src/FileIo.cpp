#include "FileIo.hpp"

#include <fstream>


namespace sorcery {
auto ReadFileBinary(std::filesystem::path const& src, std::vector<unsigned char>& out) -> bool {
  std::ifstream is{src, std::ios::in | std::ios::binary};

  if (!is.is_open()) {
    return false;
  }

  is.seekg(0, std::ios::end);
  out.resize(is.tellg());

  is.seekg(0, std::ios::beg);
  is.read(reinterpret_cast<char*>(out.data()), std::size(out));

  return true;
}
}
