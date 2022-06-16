#include "LeopphverterExport.hpp"
#include "LeopphverterImport.hpp"

#include <fstream>

auto static constexpr FILE_PATH = R"#(C:\Users\leven\Desktop\test.leopph3d)#";


auto main() -> int
{
	leopph::convert::Import(FILE_PATH);

	/*auto bytes = Export(leopph::convert::Object{});
	leopph::convert::Serialize("viharos egy geci", bytes, std::endian::little);
	std::ofstream out{FILE_PATH, std::ios::binary | std::ios::out};
	out.write(reinterpret_cast<char const*>(bytes.data()), sizeof(decltype(bytes)::value_type) * bytes.size());*/
}
