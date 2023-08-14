#pragma once

#include "Core.hpp"

#include <memory>
#include <memory_resource>
#include <new>


namespace sorcery {
class LinearMemoryResource final : public std::pmr::memory_resource {
  std::size_t mMaxByteCount;
  std::size_t mOffset;
  std::unique_ptr<char[]> mMem;


  auto do_allocate(std::size_t const requiredByteCount, std::size_t const alignment) -> void* override {
    auto ptr{static_cast<void*>(mMem.get() + mOffset)};
    auto freeByteCount{mMaxByteCount - mOffset};

    if (auto const alignedPtr{std::align(alignment, requiredByteCount, ptr, freeByteCount)}) {
      mOffset = mMaxByteCount - freeByteCount + requiredByteCount;
      return alignedPtr;
    }

    throw std::bad_alloc{};
  }


  auto do_deallocate([[maybe_unused]] void* const ptr, [[maybe_unused]] std::size_t const bytes,
                     [[maybe_unused]] std::size_t const align) -> void override { }


  [[nodiscard]] auto do_is_equal(memory_resource const& that) const noexcept -> bool override {
    if (auto const other{dynamic_cast<LinearMemoryResource const*>(&that)}) {
      return mMem == other->mMem;
    }
    return false;
  }

public:
  explicit LinearMemoryResource(std::size_t const maxByteCount) :
    mMaxByteCount{maxByteCount},
    mOffset{0},
    mMem{std::make_unique_for_overwrite<char[]>(maxByteCount)} { }


  auto Clear() noexcept -> void {
    mOffset = 0;
  }
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
