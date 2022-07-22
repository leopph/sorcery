#include "Leopphverter.hpp"
#include "3d/generic/DecodeGeneric.hpp"
#include "3d/leopph3d/DecodeLeopph3d.hpp"


namespace leopph::convert
{
	std::optional<Object> import_3d_asset(std::filesystem::path const& path)
	{
		if (path.extension() == ".leopph3d")
		{
			return decode_leopph3d(path);
		}

		return decode_generic_3d_asset(path);
	}
}
