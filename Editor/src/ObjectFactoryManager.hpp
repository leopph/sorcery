#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include <Object.hpp>

#include "ObjectWrappers.hpp"

namespace leopph::editor {
class ObjectFactoryManager {
	std::unordered_map<Object::Type, std::type_index> mEnumToTypeIdx;
	std::unordered_map<std::type_index, std::unique_ptr<BasicObjectWrapper>> mTypeIdxToWrapper;

public:
	template<typename T>
	auto Register() -> void {
		std::type_index const typeIdx{ typeid(T) };
		mEnumToTypeIdx.emplace(T::SerializationType, typeIdx);
		mTypeIdxToWrapper.emplace(typeIdx, std::make_unique<ObjectWrapper<T>>());
	}

	[[nodiscard]] BasicObjectWrapper& GetWrapperFor(Object::Type const type) const {
		return *mTypeIdxToWrapper.at(mEnumToTypeIdx.at(type));
	}

	template<typename T>
	[[nodiscard]] ObjectWrapper<T>& GetWrapperFor() const {
		return static_cast<ObjectWrapper<T>&>(*mTypeIdxToWrapper.at(typeid(T)));
	}
};
}
