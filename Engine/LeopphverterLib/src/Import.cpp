#include "LeopphverterImport.hpp"
#include "LeopphverterTypes.hpp"
#include "ParseLeopph3D.hpp"
#include "ProcessGeneric.hpp"


namespace leopph::convert
{
	auto Import(std::filesystem::path const& path) -> Object
	{
		if (path.extension() == ".leopph3d")
		{
			if (auto obj = ParseLeopph3D(path); obj.has_value())
			{
				return obj.value();
			}

			return {};
		}

		return ProcessGenericModel(path);
	}
}
