#include "Util.hpp"

#include <cctype>


namespace leopph {
auto Contains(std::string_view const src, std::string_view const target) -> bool {
	if (target.empty()) {
		return true;
	}

	if (src.empty() || target.size() > src.size()) {
		return false;
	}

	for (std::size_t i{ 0 }; i < src.size() - target.size() + 1; i++) {
		auto match{ true };

		for (std::size_t j{ 0 }; j < target.size(); j++) {
			if (std::tolower(src[i + j]) != std::tolower(target[j])) {
				match = false;
			}
		}

		if (match) {
			return true;
		}
	}

	return false;
}
}
