#include "Texture.hpp"

#include "../data/DataManager.hpp"

#include <utility>


namespace leopph::impl
{
	Texture::Texture(std::filesystem::path path) :
		m_Impl{DataManager::CreateOrGetTextureImpl(std::move(path))}
	{
		DataManager::RegisterTextureHandle(m_Impl, this);
	}

	Texture::Texture(const Texture& other) :
		m_Impl{other.m_Impl}
	{
		DataManager::RegisterTextureHandle(m_Impl, this);
	}

	Texture::Texture(Texture&& other) :
		Texture{ other }
	{}

	Texture::~Texture()
	{
		Deinit();
	}

	Texture& Texture::operator=(const Texture& other)
	{
		Deinit();
		m_Impl = other.m_Impl;
		DataManager::RegisterTextureHandle(m_Impl, this);
		return *this;
	}

	Texture& Texture::operator=(Texture&& other)
	{
		return operator=(other);
	}

	const decltype(TextureImpl::Id)& Texture::Id() const
	{
		return m_Impl->Id;
	}

	const decltype(TextureImpl::IsTransparent)& Texture::IsTransparent() const
	{
		return m_Impl->IsTransparent;
	}

	const decltype(TextureImpl::Path)& Texture::Path() const
	{
		return m_Impl->Path;
	}

	void Texture::Deinit()
	{
		if (DataManager::Textures().at(*m_Impl).size() == 1ull)
		{
			DataManager::DestroyTextureImpl(m_Impl);
		}
		else
		{
			DataManager::UnregisterTextureHandle(m_Impl, this);
		}
	}
}