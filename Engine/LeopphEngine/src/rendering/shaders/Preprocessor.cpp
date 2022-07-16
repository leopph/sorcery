#include "Logger.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>


namespace leopph::internal
{
	auto ReadFile(std::filesystem::path const& path) -> std::string
	{
		std::ifstream in{path};
		in.unsetf(decltype(in)::skipws);

		std::stringstream const s_stream;
		in >> s_stream.rdbuf();
		return s_stream.str();
	}


	auto WriteFile(std::filesystem::path const& path, std::string_view const contents) -> void
	{
		std::ofstream out{path};
		out << contents;
	}


	auto SplitLines(std::string_view const str, std::vector<std::string>& out) -> void
	{
		auto view = str;
		std::size_t end;

		do
		{
			end = view.find_first_of('\n');
			out.emplace_back(view.substr(0, end));
			view.remove_prefix(end + 1);
		}
		while (end != decltype(str)::npos);
	}


	auto ResolveIncludePath(std::filesystem::path const& includePath, std::filesystem::path const& srcDir) -> std::filesystem::path
	{
		auto ret = includePath;
		ret.make_preferred();

		if (ret.is_absolute())
		{
			return ret;
		}

		return srcDir / ret;
	}


	auto ProcessInclude(std::string& line, std::filesystem::path const& srcDir, std::unordered_map<std::string, std::string>& fileCache)
	{ }


	auto Preprocess(std::string_view const src) -> std::string
	{
		auto didProcess = true;
		std::string ret{src};

		while (didProcess)
		{
			didProcess = false;
			std::vector<std::string> lines;
			SplitLines(ret, lines);
			std::ostringstream outStream;

			for (auto i = 0; i < lines.size(); i++)
			{
				std::string_view view{lines[i]};
				auto constexpr whitespace = "\t\n\v\f\r ";
				auto whiteSpaceChars = view.find_first_of(whitespace);

				if (whiteSpaceChars == std::string_view::npos)
				{
					continue;
				}

				view.remove_prefix(whiteSpaceChars);

				if (!view.starts_with('#'))
				{
					continue;;
				}

				view.remove_prefix(1);

				whiteSpaceChars = view.find_first_of(whitespace);

				if (whiteSpaceChars == std::string_view::npos)
				{
					Logger::Instance().Error("Incomplete preprocessor directive found on line " + std::to_string(i) + '.');
					return {};
				}

				view.remove_prefix(whiteSpaceChars);

				if (view.starts_with("include"))
				{
					// Process include
					didProcess = true;
				}

				outStream << lines[i];
			}

			ret = outStream.str();
		}

		return ret;
	}
}
