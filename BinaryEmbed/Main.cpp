#include <fstream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

// argv[1]: src path
// argv[2]: dst path
// argv[3]: data name
auto main(int const argc, char* argv[]) -> int {
	if (argc <= 1) {
		std::cerr << "Please provide a source path.\n";
		return -1;
	}

	if (argc <= 2) {
		std::cerr << "Please provide a destination path.\n";
		return -1;
	}

	if (argc <= 3) {
		std::cerr << "Please provide a name for the declared variable.\n";
		return -1;
	}

	std::ifstream in{ argv[1], std::ios::binary };

	if (!in.is_open()) {
		std::cerr << "Failed to open source file.\n";
		return -1;
	}

	std::vector<unsigned char> const bytes{ std::istreambuf_iterator{ in }, {} };

	std::ofstream out{ argv[2], std::ios::binary | std::ios::trunc };

	if (out.is_open()) { }

	out << "unsigned char const " << argv[3] << "[] = {";

	for (std::size_t i{ 0 }; i < bytes.size(); i++) {
		if (i % 8 == 0) {
			out << "\n\t";
		}

		out << std::to_string(static_cast<unsigned>(bytes[i])) << ", ";
	}

	out << "\n};\n";
}
