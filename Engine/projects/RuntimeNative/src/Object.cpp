#include "Object.hpp"

#include <set>
#include <cassert>
#include <vector>


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


		//std::set<Object*, GuidObjectLess> gAllObjects;
		std::vector<Object*> gAllobjects;
	}


	Object::Object() {
		//gAllObjects.emplace(this);
		gAllobjects.emplace_back(this);
	}


	Object::~Object() {
		std::erase(gAllobjects, this);
	}


	auto Object::GetGuid() const -> Guid const& {
		return guid;
	}


	auto GetObjectWithGuid(Guid const& guid) -> Object* {
		/*if (auto const it = gAllObjects.find(guid); it != std::end(gAllObjects)) {
			return *it;
		}*/
		for (auto* const obj : gAllobjects) {
			if (obj->guid == guid) {
				return obj;
			}
		}
		return nullptr;
	}
}