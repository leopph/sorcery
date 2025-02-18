#include "command_line_processor.hpp"

#include "Platform.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <shellapi.h>

#include <cwchar>
#include <stdexcept>


namespace sorcery::mage {
CommandLineProcessor::CommandLineProcessor(wchar_t* const win_cmd_line) {
  if (!win_cmd_line) {
    throw std::invalid_argument{"Windows command line pointer is nullptr!"};
  }

  if (std::wcscmp(win_cmd_line, L"") != 0) {
    int argc;
    auto const argv{CommandLineToArgvW(win_cmd_line, &argc)};

    args_.reserve(argc);
    arg_views_.reserve(argc);

    for (auto i{0}; i < argc; i++) {
      args_.emplace_back(WideToUtf8(argv[i]));
      arg_views_.emplace_back(args_.back());
    }

    LocalFree(argv);
  }
}


CommandLineProcessor::CommandLineProcessor(int const argc, char** argv) {
  args_.reserve(argc);
  arg_views_.reserve(argc);

  for (auto i{0}; i < argc; i++) {
    args_.emplace_back(argv[i]);
    arg_views_.emplace_back(args_.back());
  }
}


auto CommandLineProcessor::GetArgs() const -> std::span<std::string_view const> {
  return arg_views_;
}
}
