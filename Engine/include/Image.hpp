#pragma once

#include "LeopphApi.hpp"

#include <cstddef>
#include <filesystem>
#include <memory>
#include <span>


namespace leopph
{
	class Image
	{
		public:
			// Parse image from file
			LEOPPHAPI
			explicit Image(std::filesystem::path src, bool flipVertically = false);

			// Create image by raw data.
			LEOPPHAPI
			Image(int width, int height, int channels, std::unique_ptr<unsigned char[]> bytes);

			// Construct and empty image.
			Image() = default;

			LEOPPHAPI
			Image(Image const& other);
			LEOPPHAPI
			auto operator=(Image const& other) -> Image&;

			LEOPPHAPI
			Image(Image&& other) noexcept;
			LEOPPHAPI
			auto operator=(Image&& other) noexcept -> Image&;

			~Image() noexcept = default;

			[[nodiscard]]
			LEOPPHAPI
			auto Path() const noexcept -> std::filesystem::path const&;

			[[nodiscard]]
			LEOPPHAPI
			auto Width() const noexcept -> int;

			[[nodiscard]]
			LEOPPHAPI
			auto Height() const noexcept -> int;

			[[nodiscard]]
			LEOPPHAPI
			auto Channels() const noexcept -> int;

			// Moves the specified channel out of the Image into a new one.
			// Channel is a zero based index.
			[[nodiscard]]
			LEOPPHAPI
			auto ExtractChannel(int channel) -> Image;

			// Returns a row.
			[[nodiscard]]
			LEOPPHAPI
			auto operator[](std::size_t rowIndex) const -> unsigned char const*;

			// Returns whether any data is held.
			[[nodiscard]]
			LEOPPHAPI
			auto Empty() const noexcept -> bool;

			// Returns the stored bytes.
			[[nodiscard]]
			LEOPPHAPI
			auto Data() const noexcept -> std::span<unsigned char const>;

		private:
			std::filesystem::path m_Path;
			int m_Width{0};
			int m_Height{0};
			int m_Channels{0};
			std::unique_ptr<unsigned char[]> m_Bytes;
	};
}
