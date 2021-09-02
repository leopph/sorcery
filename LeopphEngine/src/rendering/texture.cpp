#include "Texture.hpp"


namespace leopph::impl
{
	Texture::Texture(const std::filesystem::path& path) :
		ResourceHandle{ path }
	{}


	Texture::Texture(const Texture& other) :
		ResourceHandle{ static_cast<const ResourceHandle&>(other) }
	{}


	Texture::Texture(Texture&& other) :
		Texture{ other }
	{}


	Texture& Texture::operator=(const Texture& other)
	{
		ResourceHandle::operator=(static_cast<const ResourceHandle&>(other));
		return *this;
	}


	Texture& Texture::operator=(Texture&& other)
	{
		operator=(other);
		return *this;
	}


	const decltype(TextureResource::Id)& Texture::Id() const
	{
		return resource->Id;
	}


	const decltype(TextureResource::IsTransparent)& Texture::IsTransparent() const
	{
		return resource->IsTransparent;
	}


	const decltype(TextureResource::Path)& Texture::Path() const
	{
		return resource->Path;
	}
}