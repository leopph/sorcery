#include "sorcery/host/game_module.hpp"

#include <stdexcept>

#include <libloaderapi.h>

#include "sorcery/char_encoding_helpers.hpp"
#include "sorcery/host/platform.hpp"


namespace sorcery {
auto GameModule::Load(std::filesystem::path const& path) -> void {
  if (IsLoaded()) {
    throw std::runtime_error{"Failed to load game module: already loaded."};
  }

  module_ = LoadLibraryW(path.wstring().c_str());

  if (!module_) {
    auto const err_msg = GetLastErrorMessage();
    auto ex_msg = std::format("Failed to load game module at {}", ToUntypedSv(path.u8string()));

    if (err_msg) {
      ex_msg += std::format(": {}", WideToUtf8(*err_msg));
    }

    ex_msg += '!';

    throw std::runtime_error{ex_msg};
  }
}


auto GameModule::IsLoaded() const -> bool {
  return module_ != nullptr;
}


GameModule::~GameModule() {
  if (module_) {
    FreeLibrary(module_);
  }
}
}
