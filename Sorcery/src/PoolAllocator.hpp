#pragma once

#include <cstddef>
#include <memory>


namespace sorcery {
namespace detail {
struct FreeListNode {
  FreeListNode* next;
};
}


template<typename T> requires (sizeof(T) >= sizeof(detail::FreeListNode))
class PoolAllocator {
  std::unique_ptr<char[]> mMemory;
  detail::FreeListNode* mNextFreeBlock;

public:
  explicit PoolAllocator(std::size_t const poolSize) :
    mMemory{ std::make_unique_for_overwrite<char[]>(poolSize * sizeof(T)) },
    mNextFreeBlock{ reinterpret_cast<detail::FreeListNode*>(mMemory.get()) } {
    for (std::size_t i{ 0 }; i < poolSize - 1; i++) {
      reinterpret_cast<detail::FreeListNode&>(mMemory[i * sizeof(T)]).next = reinterpret_cast<detail::FreeListNode*>(std::addressof(mMemory[(i + 1) * sizeof(T)]));
    }
    reinterpret_cast<detail::FreeListNode&>(mMemory[(poolSize - 1) * sizeof(T)]).next = nullptr;
  }


  [[nodiscard]] auto Allocate() -> T* {
    if (!mNextFreeBlock) {
      throw std::bad_alloc{};
    }

    auto const ret{ reinterpret_cast<T*>(mNextFreeBlock) };
    mNextFreeBlock = mNextFreeBlock->next;
    return ret;
  }


  auto Deallocate(T* p) noexcept -> void {
    auto const newFreeNode{ reinterpret_cast<detail::FreeListNode*>(p) };
    newFreeNode->next = mNextFreeBlock;
    mNextFreeBlock = newFreeNode;
  }
};
}
