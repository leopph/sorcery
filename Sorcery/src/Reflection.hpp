#pragma once

#include <utility>

#include <rttr/registration>


namespace sorcery {
template<typename T, typename... Args>
[[nodiscard]] auto CreateByValue(Args... args) -> T {
  return T{ std::forward<Args>(args)... };
}


template<typename T, typename... Args>
[[nodiscard]] auto CreateByRawPtr(Args&&... args) -> T* {
  return new T{ std::forward<Args>(args)... };
}


#define REFLECT_REGISTER_CTOR_BY_VALUE(type, ...) constructor(&::sorcery::CreateByValue<type, __VA_ARGS__>)
#define REFLECT_REGISTER_CTOR_BY_RAW_PTR(type, ...) constructor(&::sorcery::CreateByRawPtr<type, __VA_ARGS__>)
}
