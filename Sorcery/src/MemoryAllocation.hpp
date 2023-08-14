#pragma once

#include "Core.hpp"

#include <memory>
#include <memory_resource>


namespace sorcery {
class LinearMemoryResource final : public std::pmr::memory_resource {
  std::size_t mMaxByteCount;
  std::size_t mOffset;
  std::unique_ptr<char[]> mMem;

  [[nodiscard]] LEOPPHAPI auto do_allocate(std::size_t requiredByteCount, std::size_t alignment) -> void* override;
  LEOPPHAPI auto do_deallocate(void* ptr, std::size_t bytes, std::size_t align) -> void override;
  [[nodiscard]] LEOPPHAPI auto do_is_equal(memory_resource const& that) const noexcept -> bool override;

public:
  LEOPPHAPI explicit LinearMemoryResource(std::size_t maxByteCount);
  LEOPPHAPI auto Clear() noexcept -> void;
};


/// \brief Get a memory resource that can be used with pmr containers.
/// The allocation will be valid for the duration of the frame only.
/// Make sure to destruct all objects allocated from this memory resource before the end of frame.
/// \return A memory resource to use in pmr containers.
[[nodiscard]] LEOPPHAPI auto GetTmpMemRes() noexcept -> LinearMemoryResource&;


template<typename T>
[[nodiscard]] auto GetTmpAlloc() noexcept -> std::pmr::polymorphic_allocator<T> {
  return std::pmr::polymorphic_allocator<T>{&GetTmpMemRes()};
}
}
