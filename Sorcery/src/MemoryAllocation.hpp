#pragma once

#include "Core.hpp"

#include <atomic>
#include <memory>
#include <memory_resource>


namespace sorcery {
class LinearMemoryResource final : public std::pmr::memory_resource {
  std::size_t max_byte_count_;
  std::atomic<std::size_t> offset_;
  std::unique_ptr<char[]> mem_;

  [[nodiscard]] LEOPPHAPI auto do_allocate(std::size_t required_byte_count, std::size_t alignment) -> void* override;
  LEOPPHAPI auto do_deallocate(void* ptr, std::size_t bytes, std::size_t align) -> void override;
  [[nodiscard]] LEOPPHAPI auto do_is_equal(memory_resource const& that) const noexcept -> bool override;

public:
  LEOPPHAPI explicit LinearMemoryResource(std::size_t max_byte_count);
  LEOPPHAPI auto Clear() noexcept -> void;
};


// A memory resource to use with pmr containers that allocates memory linearly, freeing allocations only at the end of
// the frame. The returned memory is only valid for the duration of the frame.
//LEOPPHAPI extern LinearMemoryResource g_single_frame_linear_memory; TODO temporarily disabled due to job system incompatibility


/// \brief Get a memory resource that can be used with pmr containers.
/// The allocation will be valid for the duration of the frame only.
/// Make sure to destruct all objects allocated from this memory resource before the end of frame.
/// \return A memory resource to use in pmr containers.
/*[[nodiscard]] constexpr auto GetSingleFrameLinearMemory() noexcept -> LinearMemoryResource& {
  return g_single_frame_linear_memory;
} TODO temporarily disabled due to job system incompatibility */


/*template<typename T>
[[nodiscard]] auto GetTmpAlloc() noexcept -> std::pmr::polymorphic_allocator<T> {
  return std::pmr::polymorphic_allocator<T>{&GetSingleFrameLinearMemory()};
} TODO temporarily disabled due to job system incompatibility */
}
