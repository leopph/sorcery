#pragma once

#include "Core.hpp"
#include "Util.hpp"

namespace leopph {
	struct Object {
		Guid guid{ Guid::Generate() };

		LEOPPHAPI Object();
		LEOPPHAPI virtual ~Object();

		[[nodiscard]] LEOPPHAPI auto GetGuid() const -> Guid const&;
	};


	[[nodiscard]] LEOPPHAPI auto GetObjectWithGuid(Guid const& guid) -> Object*;
}