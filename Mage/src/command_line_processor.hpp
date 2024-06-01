#pragma once

#include <span>
#include <string>
#include <string_view>
#include <vector>


namespace sorcery::mage {
class CommandLineProcessor {
public:
  explicit CommandLineProcessor(wchar_t* win_cmd_line);

  [[nodiscard]] auto GetArgs() const -> std::span<std::string_view const>;

private:
  std::vector<std::string> args_;
  std::vector<std::string_view> arg_views_;
};
}
