#pragma once

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	class Image
	{
		public:
			// Parse image from file
			explicit Image(std::filesystem::path src, bool flipVertically = false);

			// Create image by raw data.
			Image(int width, int height, int channels, std::vector<unsigned char> bytes);

			// Construct and empty image.
			Image() = default;

			Image(Image const& other);
			auto operator=(Image const& other) -> Image&;

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

			// Moves the specified channel out of the Image into a new one.
			// Channel is a zero based index.
			[[nodiscard]]
			auto ExtractChannel(int channel) -> Image;

			// Returns a row.
			[[nodiscard]]
			auto operator[](std::size_t rowIndex) const -> unsigned char const*;

			// Returns whether any data is held.
			[[nodiscard]]
			auto Empty() const noexcept -> bool;

			// Returns the stored bytes.
			[[nodiscard]]
			auto Data() const noexcept -> std::span<unsigned char const>;

		private:
			std::filesystem::path m_Path;
			int m_Width{0};
			int m_Height{0};
			int m_Channels{0};
			std::unique_ptr<unsigned char[]> m_Bytes;
	};
}
