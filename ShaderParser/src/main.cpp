#include <cstdlib>
#include <cstring>
#include <iostream>
#include <fstream>
#include <string>

#include "parse_file.h"

namespace parser
{
	const std::string stringIncludeLine{ "#include <string>\n" };
	const std::string declType{ "const std::string " };
	const std::string stringOpening{ "{ R\"#fileContents#(" };
	const std::string stringEnding{ ")#fileContents#\" };" };
}

/*--------------------------
First param: source file
Second param: target file
Third param: variable's name
Fourth param [optional]: dependency header
--------------------------*/

int main(int argc, char** argv)
{
	if (argc < 4)
	{
		std::cerr << "Invalid argument number: " << argc << "." << std::endl;
		return EXIT_FAILURE;
	}

	std::string sourceCode{ parser::ParseFile(argv[1]) };

	if (sourceCode.empty())
	{
		std::cerr << "Invalid input file: \"" << argv[1] << "\"." << std::endl;
		return EXIT_FAILURE;
	}

	std::ofstream outPut{ argv[2] };

	if (argc > 4)
	{
		std::string depIncludeLine{ "#include \"" };
		depIncludeLine.append(argv[4]);
		depIncludeLine.append("\"\n");

		outPut.write(depIncludeLine.data(), depIncludeLine.length());
	};

	outPut.write(parser::stringIncludeLine.data(), parser::stringIncludeLine.length());
	outPut.write(parser::declType.data(), parser::declType.length());
	outPut.write(argv[3], std::strlen(argv[3]));
	outPut.write(parser::stringOpening.data(), parser::stringOpening.length());
	outPut.write(sourceCode.data(), sourceCode.length());
	outPut.write(parser::stringEnding.data(), parser::stringEnding.length());

	outPut.close();

	return EXIT_SUCCESS;
}