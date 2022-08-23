#include "Util.hpp"

#include <fstream>
#include <sstream>


namespace leopph
{
	std::string read_file(std::filesystem::path const& path)
	{
		std::ifstream in{path};
		in.unsetf(decltype(in)::skipws);

		std::stringstream const s_stream;
		in >> s_stream.rdbuf();
		return s_stream.str();
	}



	std::vector<std::string>& read_file_lines(std::filesystem::path const& path, std::vector<std::string>& out)
	{
		std::ifstream in{path};
		std::string line;

		while (std::getline(in, line))
		{
			out.emplace_back(line);
		}

		return out;
	}



	void write_file(std::filesystem::path const& path, std::string_view const contents)
	{
		std::ofstream out{path};
		out << contents;
	}



	void split_lines(std::string_view const str, std::vector<std::string>& out)
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



	void split_words(std::string_view const view, std::vector<std::string_view>& out)
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



	void split_string_delim(std::string_view const str, char const sep, std::vector<std::string_view>& out)
	{
		std::size_t base = 0;

		for (std::size_t i = 0; i < str.size(); i++)
		{
			if (str[i] == sep)
			{
				out.push_back(str.substr(base, i - base));
				base = i + 1;
			}
		}

		out.push_back(str.substr(base, str.size()));
	}
}
