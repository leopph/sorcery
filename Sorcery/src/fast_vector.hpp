#pragma once

#include <vector>

#include <mimalloc.h>


namespace sorcery {
template<typename T>
using FastVector = std::vector<T, mi_stl_allocator<T>>;
}
