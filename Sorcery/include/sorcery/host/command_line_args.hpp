#pragma once

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <sorcery/Core.hpp>


namespace sorcery {
class CommandLineArgs {
public:
  SORCERYAPI
  explicit CommandLineArgs(wchar_t* win_cmd_line);
  SORCERYAPI CommandLineArgs(int argc, char** argv);

  [[nodiscard]] SORCERYAPI
  auto GetArgList() const -> std::span<std::string const>;

  [[nodiscard]] SORCERYAPI
  auto GetArg(std::string_view arg) const -> std::optional<std::string_view>;

  [[nodiscard]] SORCERYAPI
  auto HasArg(std::string_view arg) const -> bool;

private:
  std::vector<std::string> args_;
};
}
