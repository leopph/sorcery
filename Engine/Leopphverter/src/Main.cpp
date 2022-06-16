#include "LeopphverterExport.hpp"

#include <fstream>

int main()
{
	auto const bytes = Export(leopph::convert::Object{});
	std::ofstream out{R"#(C:\Users\leven\Desktop\test.bin)#", std::ios::binary | std::ios::out};
	out.write(reinterpret_cast<char const*>(bytes.data()), sizeof(decltype(bytes)::value_type) * bytes.size());
}