#include "Object.hpp"

#include <set>
#include <cassert>


namespace leopph {
	namespace {
		struct GuidObjectLess {
			using is_transparent = void;

			[[nodiscard]] auto operator()(Guid const& left, Guid const& right) const -> bool {
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

			[[nodiscard]] auto operator()(Object const* const left, Guid const& right) const -> bool {
				return operator()(left->GetGuid(), right);
			}

			[[nodiscard]] auto operator()(Guid const& left, Object* const right) const -> bool {
				return operator()(left, right->GetGuid());
			}

			[[nodiscard]] auto operator()(Object const* const left, Object const* const right) const -> bool {
				return operator()(left->GetGuid(), right->GetGuid());
			}
		};


		std::set<Object*, GuidObjectLess> gAllObjects;
	}


	Object::Object() { 
		do {
			mGuid = Guid::Generate();
		}
		while (gAllObjects.contains(this));

		gAllObjects.emplace(this);
	}


	Object::Object(Guid const& guid) : 
		mGuid{ guid } {
		assert(!gAllObjects.contains(this));
		gAllObjects.emplace(this);
	}


	auto Object::GetGuid() const -> Guid const& {
		return mGuid;
	}


	auto GetObjectWithGuid(Guid const& guid) -> Object* {
		if (auto const it = gAllObjects.find(guid); it != std::end(gAllObjects)) {
			return *it;
		}
		return nullptr;
	}
}