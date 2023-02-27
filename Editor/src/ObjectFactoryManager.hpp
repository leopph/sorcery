#pragma once

#include <unordered_map>
#include <typeinfo>
#include <typeindex>
#include <memory>

#include <Object.hpp>

#include "ObjectWrappers.hpp"

namespace leopph::editor {
class EditorObjectFactoryManager : public ObjectInstantiatorManager {
	std::unordered_map<Object::Type, std::type_index> mEnumToTypeIdx;
	std::unordered_map<std::type_index, std::unique_ptr<EditorObjectWrapper>> mTypeIdxToWrapper;

public:
	template<typename T>
	auto Register() -> void {
		std::type_index const typeIdx{ typeid(T) };
		mEnumToTypeIdx.emplace(T::SerializationType, typeIdx);
		mTypeIdxToWrapper.emplace(typeIdx, std::make_unique<EditorObjectWrapperFor<T>>());
	}

	[[nodiscard]] EditorObjectWrapper& GetFor(Object::Type const type) const override {
		return *mTypeIdxToWrapper.at(mEnumToTypeIdx.at(type));
	}

	template<typename T>
	[[nodiscard]] EditorObjectWrapperFor<T>& GetFor() const {
		return static_cast<EditorObjectWrapperFor<T>&>(*mTypeIdxToWrapper.at(typeid(T)));
	}
};
}
