#include <cctype>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_set>



std::string_view constexpr VARIABLE_DECLARATION_TYPE = "extern std::string const";
std::string_view constexpr VARIABLE_DEFINITION_TYPE = "std::string const";
std::string_view constexpr HEADER_DIRECTIVES = "#pragma once\n\n#include <string_view>";
std::string_view constexpr INCLUDE_DIRECTIVE = "#include";
std::string_view constexpr NAMESPACE_BEGIN = "namespace leopph::internal::shadersources\n{";
std::string_view constexpr NAMESPACE_END = "}";



auto GenerateVarNameFromFilename(std::filesystem::path const& filename) -> std::string;
auto FileSupported(std::filesystem::path const& filename) -> bool;
auto GenerateStringFromFileContents(std::filesystem::path const& path) -> std::string;
auto WriteFiles(std::filesystem::path const& headerPath, std::ostringstream const& headerStream, std::filesystem::path const& tuPath, std::ostringstream const& tuStream) -> void;



// argv[1] = source folder
// argv[2] = out header path
// argv[3] = out tu path
// argv[4] = generated include directory in tu
auto main(int const argc, char const* const* const argv) -> int
{
	if (argc != 5)
	{
		std::cerr << "Invalid number of command-line arguments: " << argc << ".\n";
		return -1;
	}

	try
	{
		auto const tuIncludePath = [argv]
		{
			std::ostringstream ret;
			ret << argv[4] << '/' << std::filesystem::path{argv[2]}.filename().string();
			return ret.str();
		}();

		std::ostringstream headerContents;
		std::ostringstream tuContents;

		headerContents << HEADER_DIRECTIVES << "\n\n" << NAMESPACE_BEGIN << '\n';
		tuContents << INCLUDE_DIRECTIVE << " \"" << tuIncludePath << "\"\n\n" << NAMESPACE_BEGIN << '\n';

		for (auto const& entry : std::filesystem::recursive_directory_iterator{argv[1]})
		{
			if (entry.is_regular_file() && FileSupported(entry.path().filename()))
			{
				auto const varName = GenerateVarNameFromFilename(entry.path().filename().string());
				headerContents << '\t' << VARIABLE_DECLARATION_TYPE << ' ' << varName << ";\n";
				tuContents << '\t' << VARIABLE_DEFINITION_TYPE << ' ' << varName << '{' << GenerateStringFromFileContents(entry.path()) << "};\n";
			}
		}

		headerContents << NAMESPACE_END;
		tuContents << NAMESPACE_END;
		WriteFiles(argv[2], headerContents, argv[3], tuContents);
	}
	catch (std::exception const& exception)
	{
		std::cerr << "An error occured while processing: " << exception.what() << '\n';
		return -1;
	}
}



auto GenerateVarNameFromFilename(std::filesystem::path const& filename) -> std::string
{
	auto varName = filename.stem().string();

	varName[0] = std::toupper(varName[0]);

	for (auto i = 1; i < varName.size(); i++)
	{
		if (std::isupper(varName[i]))
		{
			varName.insert(i, "_");
			i++;
		}
		else
		{
			varName[i] = std::toupper(varName[i]);
		}
	}

	auto ext = filename.extension().string().substr(1);

	for (auto& c : ext)
	{
		c = std::toupper(c);
	}

	varName.push_back('_');
	varName.append(ext);
	return varName;
}



auto FileSupported(std::filesystem::path const& filename) -> bool
{
	std::unordered_set<std::string> const supportedExts
	{
		".glsl", ".vert", ".frag"
	};
	return supportedExts.contains(filename.extension().string());
}



auto GenerateStringFromFileContents(std::filesystem::path const& path) -> std::string
{
	std::ifstream in{path};
	in.unsetf(std::ifstream::skipws);

	in.seekg(0, decltype(in)::end);
	auto const size = in.tellg();
	in.seekg(0);

	std::string buffer(file_size(path), ' ');
	in.read(buffer.data(), size);
	buffer.insert(0, "R\"(");
	buffer.append(")\"");
	return buffer;
}


auto WriteFiles(std::filesystem::path const& headerPath, std::ostringstream const& headerStream, std::filesystem::path const& tuPath, std::ostringstream const& tuStream) -> void
{
	std::ofstream out{headerPath};
	out << headerStream.view();
	out.close();
	out.open(tuPath);
	out << tuStream.view();
}
