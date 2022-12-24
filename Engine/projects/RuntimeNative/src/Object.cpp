#include "Object.hpp"

#include <set>


namespace leopph {
	std::set<Object*, Object::GuidObjectLess> Object::sAllObjects;


	auto Object::GuidObjectLess::operator()(Guid const& left, Guid const& right) const -> bool {
		auto const leftFirstHalf = *reinterpret_cast<leopph::u64 const*>(&left);
		auto const leftSecondHalf = *(reinterpret_cast<leopph::u64 const*>(&left) + 1);
		auto const rightFirstHalf = *reinterpret_cast<leopph::u64 const*>(&right);
		auto const rightSecondHalf = *(reinterpret_cast<leopph::u64 const*>(&right) + 1);

		if (leftFirstHalf < rightFirstHalf ||
			(leftFirstHalf == rightFirstHalf && leftSecondHalf < rightSecondHalf)) {
			return true;
		}
		return false;
	}


	auto Object::GuidObjectLess::operator()(Object const* const left, Guid const& right) const -> bool {
		return operator()(left->GetGuid(), right);
	}


	auto Object::GuidObjectLess::operator()(Guid const& left, Object* const right) const -> bool {
		return operator()(left, right->GetGuid());
	}


	auto Object::GuidObjectLess::operator()(Object const* const left, Object const* const right) const -> bool {
		return operator()(left->GetGuid(), right->GetGuid());
	}


	auto Object::FindObjectByGuid(Guid const& guid) -> Object* {
		if (auto const it = sAllObjects.find(guid); it != std::end(sAllObjects)) {
			return *it;
		}
		return nullptr;
	}


	Object::Object() {
		sAllObjects.emplace(this);
	}


	Object::~Object() {
		sAllObjects.erase(this);
	}


	auto Object::GetGuid() const -> Guid const& {
		return mGuid;
	}


	auto Object::SetGuid(Guid const& guid) -> void {
		auto extracted{ sAllObjects.extract(this) };
		mGuid = guid;
		sAllObjects.insert(std::move(extracted));
	}


	auto Object::GetName() const noexcept -> std::string_view {
		return mName;
	}


	auto Object::SetName(std::string name) noexcept -> void {
		mName = std::move(name);
	}
}
