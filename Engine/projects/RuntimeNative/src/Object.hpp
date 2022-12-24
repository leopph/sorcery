#pragma once

#include "Core.hpp"
#include "Util.hpp"

#include <set>
#include <string>

namespace leopph {
	class Object {
	private:
		struct GuidObjectLess {
			using is_transparent = void;
			[[nodiscard]] auto operator()(Guid const& left, Guid const& right) const -> bool;
			[[nodiscard]] auto operator()(Object const* left, Guid const& right) const -> bool;
			[[nodiscard]] auto operator()(Guid const& left, Object* right) const -> bool;
			[[nodiscard]] auto operator()(Object const* left, Object const* right) const -> bool;
		};

		static std::set<Object*, GuidObjectLess> sAllObjects;

		Guid mGuid{ Guid::Generate() };
		std::string mName;

	public:
		[[nodiscard]] LEOPPHAPI static auto FindObjectByGuid(Guid const& guid) -> Object*;

		LEOPPHAPI Object();
		LEOPPHAPI virtual ~Object();

		[[nodiscard]] LEOPPHAPI auto GetGuid() const->Guid const&;
		LEOPPHAPI auto SetGuid(Guid const& guid) -> void;

		[[nodiscard]] LEOPPHAPI auto GetName() const noexcept -> std::string_view;
		LEOPPHAPI auto SetName(std::string name) noexcept -> void;
	};



}