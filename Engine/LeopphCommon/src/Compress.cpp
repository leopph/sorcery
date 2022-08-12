#include "Compress.hpp"

#include <iostream>
#include <iterator>
#include <memory>
#include <zlib.h>


namespace leopph
{
	namespace
	{
		constexpr auto COMPR_LVL = Z_BEST_COMPRESSION;
		constexpr auto TMP_BUF_SZ = 256 * 1024;
	}



	CompressionError compress(std::span<u8> in, std::vector<u8>& out)
	{
		auto const tmpBuf = std::make_unique_for_overwrite<u8[]>(TMP_BUF_SZ);

		z_stream stream;
		stream.zalloc = nullptr;
		stream.zfree = nullptr;
		stream.opaque = nullptr;
		stream.avail_in = static_cast<uInt>(in.size_bytes());
		stream.next_in = in.data();
		stream.avail_out = TMP_BUF_SZ;
		stream.next_out = tmpBuf.get();

		deflateInit(&stream, COMPR_LVL);

		auto flush = Z_NO_FLUSH;

		while (true)
		{
			auto const deflateRet = deflate(&stream, flush);

			if (deflateRet == Z_STREAM_ERROR)
			{
				deflateEnd(&stream);
				return CompressionError::Inconsistency;
			}

			if (deflateRet == Z_BUF_ERROR)
			{
				if (stream.avail_out == 0)
				{
					std::copy_n(tmpBuf.get(), TMP_BUF_SZ, std::back_inserter(out));
					stream.avail_out = TMP_BUF_SZ;
					stream.next_out = tmpBuf.get();
					continue;
				}

				if (stream.avail_in == 0)
				{
					flush = Z_FINISH;
					continue;
				}

				deflateEnd(&stream);
				return CompressionError::Unknown;
			}

			if (deflateRet == Z_STREAM_END)
			{
				std::copy_n(tmpBuf.get(), TMP_BUF_SZ - stream.avail_out, std::back_inserter(out));
				deflateEnd(&stream);
				return CompressionError::None;
			}
		}
	}



	CompressionError uncompress(std::span<u8> in, u64 const uncompressedSize, std::vector<u8>& out)
	{
		// Make sure the vector has enough space
		out.resize(out.size() + uncompressedSize);

		z_stream stream;
		stream.zalloc = nullptr;
		stream.zfree = nullptr;
		stream.opaque = nullptr;
		stream.avail_in = static_cast<uInt>(in.size_bytes());
		stream.next_in = in.data();
		stream.avail_out = static_cast<uInt>(uncompressedSize);
		stream.next_out = &out[out.size() - uncompressedSize];

		inflateInit(&stream);
		auto const inflateRet = inflate(&stream, Z_FINISH);
		inflateEnd(&stream);

		switch (inflateRet)
		{
			case Z_STREAM_END:
			{
				return CompressionError::None;
			}

			case Z_STREAM_ERROR:
			{
				return CompressionError::Inconsistency;
			}

			default:
			{
				return CompressionError::Unknown;
			}
		}
	}
}
