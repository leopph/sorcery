#include "MemoryAllocation.hpp"

#include <new>


namespace sorcery {
LinearMemoryResource g_single_frame_linear_memory{static_cast<std::size_t>(128) * 1024 * 1024};


auto LinearMemoryResource::do_allocate(std::size_t const required_byte_count, std::size_t const alignment) -> void* {
  while (true) {
    auto offset{offset_.load()};
    auto ptr{static_cast<void*>(mem_.get() + offset)};
    auto free_byte_count{max_byte_count_ - offset};

    if (!std::align(alignment, required_byte_count, ptr, free_byte_count)) {
      throw std::bad_alloc{};
    }

    if (offset_.compare_exchange_weak(offset, max_byte_count_ - free_byte_count + required_byte_count)) {
      return ptr;
    }
  }
}


auto LinearMemoryResource::do_deallocate([[maybe_unused]] void* const ptr, [[maybe_unused]] std::size_t const bytes,
                                         [[maybe_unused]] std::size_t const align) -> void {}


auto LinearMemoryResource::do_is_equal(memory_resource const& that) const noexcept -> bool {
  if (auto const other{dynamic_cast<LinearMemoryResource const*>(&that)}) {
    return mem_ == other->mem_;
  }
  return false;
}


LinearMemoryResource::LinearMemoryResource(std::size_t const max_byte_count):
  max_byte_count_{max_byte_count},
  offset_{0},
  mem_{std::make_unique_for_overwrite<char[]>(max_byte_count)} {}


auto LinearMemoryResource::Clear() noexcept -> void {
  offset_ = 0;
}
}
