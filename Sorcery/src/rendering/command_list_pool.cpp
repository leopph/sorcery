#include "command_list_pool.hpp"


namespace sorcery::rendering {
auto CommandListPool::Acquire() -> wand::CommandList& {
  std::scoped_lock const lck{mutex_};

  if (next_cmd_list_idx_ >= cmd_lists_.size()) {
    cmd_lists_.emplace_back(device_->CreateCommandList());
  }

  return *cmd_lists_[next_cmd_list_idx_++];
}


auto CommandListPool::Reset() -> void {
  std::scoped_lock const lck{mutex_};
  next_cmd_list_idx_ = 0;
}


CommandListPool::CommandListPool(wand::GraphicsDevice& device) :
  device_{&device} {}
}
