#include "Parse.hpp"

#include "Logger.hpp"

#include <cstddef>
#include <fstream>
#include <vector>


namespace leopph::convert
{
	namespace
	{
		
	}

	auto ParseLeopph3D(std::filesystem::path const& path) -> std::optional<Object>
	{
		std::fstream in{path, std::ios::in | std::ios::binary};

		if (!in.is_open())
		{
			internal::Logger::Instance().Error("Error while parsing leopph3d: the file does not exist.");
			return {};
		}

		std::vector<std::uint8_t> buffer(5);

		in.read(reinterpret_cast<char*>(buffer.data()), 5);

		if (in.eof() || in.fail())
		{
			internal::Logger::Instance().Error("Error while parsing leopph3d: the file is not in valid leopph3d format.");
			return {};
		}

		std::copy(std::istream_iterator<std::uint8_t>(in), std::istream_iterator<std::uint8_t>(), std::back_inserter(buffer));



		return {};
	}
}
