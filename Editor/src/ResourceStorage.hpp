#pragma once

#include <unordered_map>
#include <filesystem>
#include <memory>

#include "Object.hpp"

namespace leopph::editor {
class ResourceStorage {
	std::unordered_map<std::filesystem::path, std::shared_ptr<Object>> mData;

public:
	auto operator[](std::filesystem::path const& path) -> decltype(auto) {
		return mData[absolute(path)];
	}

	[[nodiscard]] auto find(std::filesystem::path const& path) const -> decltype(auto) {
		return mData.find(path);
	}

	[[nodiscard]] auto begin() const noexcept -> decltype(auto) {
		return std::begin(mData);
	}

	[[nodiscard]] auto end() const noexcept -> decltype(auto) {
		return std::end(mData);
	}

	[[nodiscard]] auto cbegin() const noexcept -> decltype(auto) {
		return std::cbegin(mData);
	}

	[[nodiscard]] auto cend() const noexcept -> decltype(auto) {
		return std::cend(mData);
	}

	auto clear() noexcept -> decltype(auto) {
		mData.clear();
	}
};
}
