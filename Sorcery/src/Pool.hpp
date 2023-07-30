#pragma once

#include "PoolAllocator.hpp"


namespace sorcery {
template<typename T>
class Pool {
  PoolAllocator<T> mAlloc;

public:
  explicit Pool(std::size_t const size) :
    mAlloc{ size } {}


  template<typename... Args>
  auto New(Args&&... args) -> T* {
    auto const p{ mAlloc.Allocate() };

    if (!p) {
      return nullptr;
    }

    return new(p)T{ std::forward<Args>(args)... };
  }


  auto Delete(T* const p) -> void {
    p->~T();
    mAlloc.Deallocate(p);
  }
};
}
