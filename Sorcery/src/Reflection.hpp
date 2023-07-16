#pragma once

#include <utility>
#include <memory>

#include <rttr/registration>


namespace sorcery {
template<typename T, typename... Args>
[[nodiscard]] auto CreateByValue(Args... args) -> T {
  return T{ std::forward<Args>(args)... };
}


template<typename T, typename... Args>
[[nodiscard]] auto CreateByUniquePtr(Args&&... args) -> std::unique_ptr<T> {
  return std::unique_ptr<T>{ new T{ std::forward<Args>(args)... } };
}


#define REFLECT_REGISTER_CTOR_BY_VALUE(type, ...) constructor(&::sorcery::CreateByValue<type, __VA_ARGS__>)
#define REFLECT_REGISTER_CTOR_BY_UNIQUE_PTR(type, ...) constructor(&::sorcery::CreateByUniquePtr<type, __VA_ARGS__>)
}
