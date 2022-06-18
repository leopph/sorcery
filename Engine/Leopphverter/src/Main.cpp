#include "LeopphverterExport.hpp"
#include "LeopphverterImport.hpp"

#include <array>
#include <fstream>

static constexpr std::array SRC_FILE_PATHS
{
	R"#(C:\Dev\LeopphEngine\Client\models\church\ChristchurchGreyfriarsRuinGarden03.obj)#",
	R"#(C:\Dev\LeopphEngine\Client\models\lamp\scene.gltf)#"
};

static constexpr std::array DST_FILE_PATHS
{
	R"#(C:\Dev\LeopphEngine\Client\models\church\church.leopph3d)#",
	R"#(C:\Dev\LeopphEngine\Client\models\lamp\lamp.leopph3d)#"
};


auto main() -> int
{
	for (std::size_t i = 0; i < SRC_FILE_PATHS.size() && i < DST_FILE_PATHS.size(); i++)
	{
		auto const model = leopph::convert::Import(SRC_FILE_PATHS[i]);
		auto bytes = leopph::convert::Export(model, std::endian::little);

		std::ofstream out{DST_FILE_PATHS[i], std::ios::binary | std::ios::out};
		out.setf(std::ios_base::unitbuf);
		out.write(reinterpret_cast<char const*>(bytes.data()), sizeof(decltype(bytes)::value_type) * bytes.size());
	}
}
