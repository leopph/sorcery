#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include "Object.hpp"
#include "ObjectWrappers.hpp"
#include "Util.hpp"


namespace sorcery::mage {
class ObjectWrapperManager : public ObjectInstantiatorManager {
  std::unordered_map<Object::Type, std::type_index> mEnumToTypeIdx;
  std::unordered_map<std::type_index, std::unique_ptr<ObjectWrapper>> mTypeIdxToWrapper;

  ObjectWrapperManager() = default;

public:
  [[nodiscard]] static auto Create() noexcept -> std::shared_ptr<ObjectWrapperManager>;


  template<typename T>
  auto Register() -> void {
    std::type_index const typeIdx{ typeid(T) };
    mEnumToTypeIdx.emplace(T::SerializationType, typeIdx);
    mTypeIdxToWrapper.emplace(typeIdx, std::make_unique<ObjectWrapperFor<T>>());
  }


  [[nodiscard]] ObjectWrapper& GetFor(Object::Type const type) const override {
    if (auto const typeIdxIt{ mEnumToTypeIdx.find(type) }; typeIdxIt != std::end(mEnumToTypeIdx)) {
      if (auto const wrapperIt{ mTypeIdxToWrapper.find(typeIdxIt->second) }; wrapperIt != std::end(mTypeIdxToWrapper)) {
        return *wrapperIt->second;
      }
    }

    throw std::runtime_error{ "Couldn't find object wrapper for the requested type." };
  }


  template<typename T>
  [[nodiscard]] ObjectWrapperFor<T>& GetFor() const {
    return static_cast<ObjectWrapperFor<T>&>(GetFor(T::SerializationType));
  }


  auto GetWrappers(std::vector<ObserverPtr<ObjectWrapper>>& outWrappers) const -> std::vector<ObserverPtr<ObjectWrapper>>&;
};
}
