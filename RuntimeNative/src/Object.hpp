#pragma once

#include "Core.hpp"
#include "Guid.hpp"
#include "Util.hpp"

#include <concepts>
#include <set>
#include <string>
#include <vector>

namespace leopph {
class Object {
	struct GuidObjectLess {
		using is_transparent = void;
		[[nodiscard]] auto operator()(Guid const& left, Guid const& right) const -> bool;
		[[nodiscard]] auto operator()(Object const* left, Guid const& right) const -> bool;
		[[nodiscard]] auto operator()(Guid const& left, Object const* right) const -> bool;
		[[nodiscard]] auto operator()(Object const* left, Object const* right) const -> bool;
	};

	LEOPPHAPI static std::set<Object*, GuidObjectLess> sAllObjects;

	Guid mGuid{ Guid::Generate() };
	std::string mName{ "New Object" };

public:
	enum class Type {
		Entity,
		Transform,
		Camera,
		Behavior,
		Model,
		Light,
		Material,
		Mesh,
		Scene,
		Texture2D,
		Cubemap
	};

	[[nodiscard]] LEOPPHAPI static auto FindObjectByGuid(Guid const& guid) -> Object*;

	template<std::derived_from<Object> T>
	[[nodiscard]] static auto FindObjectOfType() -> T*;

	template<std::derived_from<Object> T>
	static auto FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>&;

	template<std::derived_from<Object> T>
	[[nodiscard]] auto FindObjectsOfType() -> std::vector<T*>;

	template<std::derived_from<Object> T>
	static auto FindObjectsOfType(std::vector<Object*>& out) -> std::vector<Object*>&;

	LEOPPHAPI Object();
	Object(Object const& other) = delete;
	Object(Object&& other) = delete;

	LEOPPHAPI virtual ~Object();

	auto operator=(Object const& other) -> void = delete;
	auto operator=(Object&& other) -> void = delete;

	[[nodiscard]] LEOPPHAPI auto GetGuid() const -> Guid const&;
	LEOPPHAPI auto SetGuid(Guid const& guid) -> void;

	[[nodiscard]] LEOPPHAPI auto GetName() const noexcept -> std::string_view;
	LEOPPHAPI auto SetName(std::string name) noexcept -> void;

	[[nodiscard]] virtual auto GetSerializationType() const -> Type = 0;
};


class ObjectInstantiator {
public:
	ObjectInstantiator() = default;
	ObjectInstantiator(ObjectInstantiator const& other) = default;
	ObjectInstantiator(ObjectInstantiator&& other) noexcept = default;

	virtual ~ObjectInstantiator() = default;

	auto operator=(ObjectInstantiator const& other) -> ObjectInstantiator& = default;
	auto operator=(ObjectInstantiator&& other) noexcept -> ObjectInstantiator& = default;

	[[nodiscard]] virtual auto Instantiate() -> Object* = 0;
};


class ObjectInstantiatorManager {
public:
	ObjectInstantiatorManager() = default;
	ObjectInstantiatorManager(ObjectInstantiatorManager const& other) = default;
	ObjectInstantiatorManager(ObjectInstantiatorManager&& other) noexcept = default;

	virtual ~ObjectInstantiatorManager() = default;

	auto operator=(ObjectInstantiatorManager const& other) -> ObjectInstantiatorManager& = default;
	auto operator=(ObjectInstantiatorManager&& other) noexcept -> ObjectInstantiatorManager& = default;

	[[nodiscard]] virtual auto GetFor(Object::Type type) const -> ObjectInstantiator& = 0;
};


template<std::derived_from<Object> T>
auto Object::FindObjectOfType() -> T* {
	for (auto const obj : sAllObjects) {
		if (auto const castObj{ dynamic_cast<T*>(obj) }; castObj) {
			return castObj;
		}
	}
	return nullptr;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<T*>& out) -> std::vector<T*>& {
	out.clear();
	for (auto const obj : sAllObjects) {
		if (auto const castObj{ dynamic_cast<T*>(obj) }; castObj) {
			out.emplace_back(castObj);
		}
	}
	return out;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType() -> std::vector<T*> {
	std::vector<T*> ret;
	FindObjectsOfType<T>(ret);
	return ret;
}


template<std::derived_from<Object> T>
auto Object::FindObjectsOfType(std::vector<Object*>& out) -> std::vector<Object*>& {
	out.clear();
	for (auto const obj : sAllObjects) {
		if (dynamic_cast<T*>(obj)) {
			out.emplace_back(obj);
		}
	}
	return out;
}
}
