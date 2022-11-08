#pragma once

#include "Core.hpp"
#include "Util.hpp"

namespace leopph {
	class Object {
	private:
		GUID mGuid;

	public:
		LEOPPHAPI Object();
		LEOPPHAPI explicit Object(GUID const& guid);
		virtual ~Object() = default;

		[[nodiscard]] LEOPPHAPI auto GetGuid() const -> GUID const&;
	};


	[[nodiscard]] LEOPPHAPI auto GetObjectWithGuid(GUID const& guid) -> Object*;
}