#include "Resource.hpp"

namespace leopph {
	auto Resource::GetSharedPtr() -> std::shared_ptr<Resource> {
		return shared_from_this();
	}

	auto Resource::GetSharedPtr() const -> std::shared_ptr<Resource const> {
		return shared_from_this();
	}
}
