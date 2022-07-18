#include "Util.hpp"

#include <fstream>
#include <sstream>

namespace leopph
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

	auto SplitWords(std::string_view const view, std::vector<std::string_view>& out) -> void
	{
		auto localView = view;

		while (true)
		{
			auto constexpr whiteSpaceChars = "\t\n\v\f\r ";
			auto const numWhiteSpace = localView.find_first_not_of(whiteSpaceChars);

			if (numWhiteSpace == std::string_view::npos)
			{
				return;
			}

			localView.remove_prefix(numWhiteSpace);

			auto const numUsefulChars = localView.find_first_of(whiteSpaceChars);

			if (numUsefulChars == std::string_view::npos)
			{
				out.emplace_back(view);
				return;
			}

			out.emplace_back(localView.substr(0, numUsefulChars));
			localView.remove_prefix(numUsefulChars);
		}
	}
}
