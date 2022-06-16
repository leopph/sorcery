#include "LeopphverterExport.hpp"
#include "LeopphverterImport.hpp"

#include <fstream>

auto static constexpr SRC_FILE_PATH = R"#(C:\Users\leven\Downloads\books\model.obj)#";
auto static constexpr DST_FILE_PATH = R"#(C:\Users\leven\Desktop\test.leopph3d)#";


auto main() -> int
{
	auto const model = leopph::convert::Import(SRC_FILE_PATH);
	auto bytes = leopph::convert::Export(model, std::endian::little);
	std::ofstream out{DST_FILE_PATH, std::ios::binary | std::ios::out};
	out.write(reinterpret_cast<char const*>(bytes.data()), sizeof(decltype(bytes)::value_type) * bytes.size());
}
