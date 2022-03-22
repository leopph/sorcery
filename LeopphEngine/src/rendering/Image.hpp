#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <vector>


namespace leopph
{
	class Image
	{
		public:
			// Parse image from file
			explicit Image(std::filesystem::path src, bool flipVertically = false);

			Image(Image const& other) = default;
			auto operator=(Image const& other) -> Image& = default;

			Image(Image&& other) noexcept;
			auto operator=(Image&& other) noexcept -> Image&;

			~Image() noexcept = default;

			[[nodiscard]]
			auto Path() const noexcept -> std::filesystem::path const&;

			[[nodiscard]]
			auto Width() const noexcept -> int;

			[[nodiscard]]
			auto Height() const noexcept -> int;

			[[nodiscard]]
			auto Channels() const noexcept -> int;

			// Returns a row.
			[[nodiscard]]
			auto operator[](std::size_t rowIndex) const -> unsigned char const*;

		private:
			std::filesystem::path m_Path;
			int m_Width{0};
			int m_Height{0};
			int m_Channels{0};
			std::vector<unsigned char> m_Bytes;
	};
}


template<>
struct std::hash<leopph::Image>
{
	auto operator()(leopph::Image const& img) const noexcept -> std::size_t;
};
