#include "LeopphverterCommon.hpp"
#include "LeopphverterImport.hpp"
#include "Parse.hpp"
#include "Process.hpp"


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
