#include "io_helpers.hpp"

#include <fstream>


namespace sorcery {
namespace {
template<typename T>
[[nodiscard]]
auto ReadBinaryFileHelper(
  std::filesystem::path const& src,
  std::vector<T>& out
) -> bool {
  std::ifstream file{src, std::ios::binary | std::ios::ate};

  if (!file) {
    return false;
  }

  auto const file_size{file.tellg()};

  if (file_size < 0) {
    return false;
  }

  out.resize(static_cast<std::size_t>(file_size));

  file.seekg(0);

  if (!file.read(reinterpret_cast<char*>(out.data()), static_cast<std::streamsize>(out.size()))) {
    return false;
  }

  return true;
}
}


auto ReadBinaryFile(
  std::filesystem::path const& src,
  std::vector<std::byte>& out
) -> bool {
  return ReadBinaryFileHelper(src, out);
}


auto ReadBinaryFile(
  std::filesystem::path const& src,
  std::vector<unsigned char>& out
) -> bool {
  return ReadBinaryFileHelper(src, out);
}
}
