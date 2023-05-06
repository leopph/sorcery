#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

// argv[1]: src path
// argv[2]: dst path
// argv[3]: data name
auto main(int const argc, char* argv[]) -> int {
  if (argc <= 1) {
    std::cerr << "Please provide a source path.\n";
    return -1;
  }

  if (argc <= 2) {
    std::cerr << "Please provide a destination path.\n";
    return -1;
  }

  if (argc <= 3) {
    std::cerr << "Please provide a name for the declared variable.\n";
    return -1;
  }

  std::filesystem::path const srcPath{ argv[1] };

  if (!exists(srcPath)) {
    std::cerr << "The provided source path does not exist.\n";
    return -1;
  }

  std::filesystem::path const dstPath{ argv[2] };

  if (exists(dstPath) && last_write_time(srcPath) < last_write_time(dstPath)) {
    return 0;
  }

  std::ifstream in{ srcPath, std::ios::binary };

  if (!in.is_open()) {
    std::cerr << "Failed to open source file.\n";
    return -1;
  }

  std::vector<unsigned char> const bytes{ std::istreambuf_iterator{ in }, {} };

  in.close();

  std::ofstream out{ dstPath, std::ios::binary | std::ios::trunc };

  if (!out.is_open()) {
    std::cerr << "Failed to open destination file.\n";
    return -1;
  }

  out << "#pragma once\n\nunsigned char const " << argv[3] << "[] = {";

  for (std::size_t i{ 0 }; i < bytes.size(); i++) {
    if (i % 8 == 0) {
      out << "\n\t";
    }

    out << std::to_string(static_cast<unsigned>(bytes[i])) << ", ";
  }

  out << "\n};\n";
}
