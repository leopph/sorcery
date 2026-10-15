#pragma once

#include <filesystem>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <sorcery/Core.hpp>


namespace sorcery {
class GameModule {
public:
  SORCERYAPI
  auto Load(std::filesystem::path const& path) -> void;

  [[nodiscard]] SORCERYAPI
  auto IsLoaded() const -> bool;

  GameModule() = default;
  GameModule(GameModule const& other) = delete;
  GameModule(GameModule&& other) noexcept = delete;

  SORCERYAPI
  ~GameModule();

  auto operator=(GameModule const& other) -> GameModule& = delete;
  auto operator=(GameModule&& other) noexcept -> GameModule& = delete;

private:
  HMODULE module_{nullptr};
};
}
