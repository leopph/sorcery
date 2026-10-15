#include "sorcery/host/command_line_args.hpp"

#include <algorithm>
#include <cwchar>
#include <format>
#include <stdexcept>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <shellapi.h>

#include "sorcery/host/platform.hpp"


namespace sorcery {
CommandLineArgs::CommandLineArgs(wchar_t* const win_cmd_line) {
  if (!win_cmd_line) {
    throw std::invalid_argument{"Windows command line pointer is nullptr!"};
  }

  if (std::wcscmp(win_cmd_line, L"") != 0) {
    int argc;
    auto const argv{CommandLineToArgvW(win_cmd_line, &argc)};

    args_.reserve(argc);

    for (auto i{0}; i < argc; i++) {
      args_.emplace_back(WideToUtf8(argv[i]));
    }

    LocalFree(argv);
  }
}


CommandLineArgs::CommandLineArgs(int const argc, char** argv) {
  args_.reserve(argc);

  for (auto i{0}; i < argc; i++) {
    args_.emplace_back(argv[i]);
  }
}


auto CommandLineArgs::GetArgList() const -> std::span<std::string const> {
  return args_;
}


auto CommandLineArgs::GetArg(std::string_view const arg) const -> std::optional<std::string_view> {
  std::string const arg_prefix{std::format("--{}=", arg)};

  for (auto const& stored_arg : args_) {
    if (stored_arg.starts_with(arg_prefix)) {
      return std::string_view{stored_arg}.substr(arg_prefix.size());
    }
  }

  return std::nullopt;
}


auto CommandLineArgs::HasArg(std::string_view const arg) const -> bool {
  return std::ranges::any_of(args_, [arg_prefix = std::format("--{}", arg)](std::string_view const view) {
    return view == arg_prefix || view.starts_with(arg_prefix + "=");
  });
}
}
