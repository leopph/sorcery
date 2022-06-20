#include "LeopphverterImport.hpp"
#include "LeopphverterTypes.hpp"
#include "ParseLeopph3D.hpp"
#include "ProcessGeneric.hpp"


namespace leopph::convert
{
	auto Import(std::filesystem::path const& path) -> std::optional<Object>
	{
		if (path.extension() == ".leopph3d")
		{
			return ParseLeopph3D(path);
		}

		return ProcessGenericModel(path);
	}
}
