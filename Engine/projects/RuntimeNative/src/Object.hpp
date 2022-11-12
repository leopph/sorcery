#pragma once

#include "Core.hpp"
#include "Util.hpp"

#include <set>

namespace leopph {
	class Object {
	private:
		struct GuidObjectLess {
			using is_transparent = void;
			[[nodiscard]] auto operator()(Guid const& left, Guid const& right) const -> bool;
			[[nodiscard]] auto operator()(Object const* const left, Guid const& right) const -> bool;
			[[nodiscard]] auto operator()(Guid const& left, Object* const right) const -> bool;
			[[nodiscard]] auto operator()(Object const* const left, Object const* const right) const -> bool;
		};
		static std::set<Object*, GuidObjectLess> sAllObjects;
		Guid mGuid{ Guid::Generate() };

	public:
		[[nodiscard]] LEOPPHAPI static auto FindObjectByGuid(Guid const& guid) -> Object*;

		LEOPPHAPI Object();
		LEOPPHAPI virtual ~Object();

		[[nodiscard]] LEOPPHAPI auto GetGuid() const->Guid const&;
		LEOPPHAPI auto SetGuid(Guid const& guid) -> void;
	};



}