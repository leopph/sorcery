#pragma once

#include "Core.hpp"
#include "Util.hpp"

namespace leopph {
	class Object {
	private:
		Guid mGuid;

	public:
		LEOPPHAPI Object();
		LEOPPHAPI explicit Object(Guid const& guid);
		virtual ~Object() = default;

		[[nodiscard]] LEOPPHAPI auto GetGuid() const -> Guid const&;
	};


	[[nodiscard]] LEOPPHAPI auto GetObjectWithGuid(Guid const& guid) -> Object*;
}