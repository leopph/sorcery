#pragma once

#include "Object.hpp"

#include <memory>
#include <vector>


namespace leopph {
	class Resource : public Object, public std::enable_shared_from_this<Resource> {
	private:
		template<std::derived_from<Resource> T>
		[[nodiscard]] static auto GetPtrHolder() -> std::vector<T*>&;

	public:
		[[nodiscard]] LEOPPHAPI auto GetSharedPtr() ->std::shared_ptr<Resource>;
		[[nodiscard]] LEOPPHAPI auto GetSharedPtr() const->std::shared_ptr<Resource const>;

		template<std::derived_from<Resource> T>
		[[nodiscard]] static auto FindResourceOfType() -> std::shared_ptr<T>;

		template<std::derived_from<Resource> T>
		static auto FindResourcesOfType(std::vector<std::shared_ptr<T>>& out) -> std::vector<std::shared_ptr<T>>&;

		template<std::derived_from<Resource> T>
		[[nodiscard]] auto FindResourcesOfType() -> std::vector<std::shared_ptr<T>>;

		template<std::derived_from<Resource> T>
		static auto FindResourcesOfType(std::vector<std::shared_ptr<Resource>>& out) -> std::vector<std::shared_ptr<Resource>>&;
	};


	template<std::derived_from<Resource> T>
	auto Resource::GetPtrHolder() -> std::vector<T*>& {
		std::vector<T*> static holder;
		return holder;
	}


	template<std::derived_from<Resource> T>
	auto Resource::FindResourceOfType() -> std::shared_ptr<T> {
		return FindObjectOfType<T>()->GetSharedPtr();
	}


	template<std::derived_from<Resource> T>
	auto Resource::FindResourcesOfType(std::vector<std::shared_ptr<T>>& out) -> std::vector<std::shared_ptr<T>>& {
		auto& tmp{ GetPtrHolder<T>() };
		FindObjectsOfType(tmp);

		out.clear();
		for (auto const ptr : tmp) {
			out.emplace_back(ptr->GetSharedPtr());
		}
		return out;
	}


	template<std::derived_from<Resource> T>
	auto Resource::FindResourcesOfType() -> std::vector<std::shared_ptr<T>> {
		auto& tmp{ GetPtrHolder<T>() };
		FindObjectsOfType(tmp);

		std::vector<std::shared_ptr<T>> out;
		out.reserve(tmp.size());
		for (auto const ptr : tmp) {
			out.emplace_back(ptr->GetSharedPtr());
		}
		return out;
	}


	template<std::derived_from<Resource> T>
	auto Resource::FindResourcesOfType(std::vector<std::shared_ptr<Resource>>& out) -> std::vector<std::shared_ptr<Resource>>& {
		std::vector<Object*> static tmp;
		FindObjectsOfType<T>(tmp);

		out.clear();
		for (auto const ptr : tmp) {
			out.emplace_back(static_cast<T*>(ptr)->GetSharedPtr());
		}
		return out;
	}
}
