#include "MemoryAllocation.hpp"

#include <new>


namespace sorcery {
namespace {
LinearMemoryResource gLinMemRes{static_cast<std::size_t>(128) * 1024 * 1024};
}


auto LinearMemoryResource::do_allocate(std::size_t const requiredByteCount, std::size_t const alignment) -> void* {
  auto ptr{static_cast<void*>(mMem.get() + mOffset)};
  auto freeByteCount{mMaxByteCount - mOffset};

  if (auto const alignedPtr{std::align(alignment, requiredByteCount, ptr, freeByteCount)}) {
    mOffset = mMaxByteCount - freeByteCount + requiredByteCount;
    return alignedPtr;
  }

  throw std::bad_alloc{};
}


auto LinearMemoryResource::do_deallocate([[maybe_unused]] void* const ptr, [[maybe_unused]] std::size_t const bytes, [[maybe_unused]] std::size_t const align) -> void { }


auto LinearMemoryResource::do_is_equal(memory_resource const& that) const noexcept -> bool {
  if (auto const other{dynamic_cast<LinearMemoryResource const*>(&that)}) {
    return mMem == other->mMem;
  }
  return false;
}


LinearMemoryResource::LinearMemoryResource(std::size_t const maxByteCount):
  mMaxByteCount{maxByteCount},
  mOffset{0},
  mMem{std::make_unique_for_overwrite<char[]>(maxByteCount)} { }


auto LinearMemoryResource::Clear() noexcept -> void {
  mOffset = 0;
}


auto GetTmpMemRes() noexcept -> LinearMemoryResource& {
  return gLinMemRes;
}
}
