#include "textureequal.h"

bool leopph::impl::TextureEqual::operator()(const TextureReference& left, const TextureReference& right) const
{
	return left.path == right.path;
}

bool leopph::impl::TextureEqual::operator()(const TextureReference& left, const decltype(TextureReference::id)& right) const
{
	return left.id == right;
}

bool leopph::impl::TextureEqual::operator()(const decltype(TextureReference::id)& left, const TextureReference& right) const
{
	return left == right.id;
}

bool leopph::impl::TextureEqual::operator()(const TextureReference& left, const std::filesystem::path& right) const
{
	return left.path == right;
}

bool leopph::impl::TextureEqual::operator()(const std::filesystem::path& left, const TextureReference& right) const
{
	return left == right.path;
}
