#pragma once

#include <cstddef>
#include <mutex>
#include <vector>

#include "../observer_ptr.hpp"
#include "wand/wand.hpp"


namespace sorcery::rendering {
class CommandListPool {
public:
  [[nodiscard]]
  auto Acquire() -> wand::CommandList&;

  auto Reset() -> void;

  explicit CommandListPool(wand::GraphicsDevice& device);
  CommandListPool(CommandListPool const& other) = delete;
  CommandListPool(CommandListPool&& other) noexcept = delete;

  ~CommandListPool() = default;

  auto operator=(CommandListPool const& other) -> CommandListPool& = delete;
  auto operator=(CommandListPool&& other) noexcept -> CommandListPool& = delete;

private:
  std::vector<wand::SharedDeviceHandle<wand::CommandList>> cmd_lists_;
  std::mutex mutex_;
  ObserverPtr<wand::GraphicsDevice> device_;
  std::size_t next_cmd_list_idx_{0};
};
}
