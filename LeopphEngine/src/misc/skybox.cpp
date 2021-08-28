#include "skybox.h"

#include "../instances/DataManager.hpp"

#include <stdexcept>

namespace leopph
{
	Skybox::Skybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front) :
		m_Impl{ impl::DataManager::GetSkybox(left, right, top, bottom, back, front) }
	{
		if (m_Impl == nullptr)
			m_Impl = impl::DataManager::RegisterSkybox(left, right, top, bottom, back, front);
		else
			impl::DataManager::IncSkybox(m_Impl);
	}

	Skybox::Skybox(const Skybox& other) :
		m_Impl{ other.m_Impl }
	{
		impl::DataManager::IncSkybox(m_Impl);
	}

	Skybox::Skybox(Skybox&& other) noexcept :
		Skybox{ other }
	{}

	Skybox::~Skybox()
	{
		impl::DataManager::DecSkybox(m_Impl);
	}

	LEOPPHAPI const std::string& Skybox::FileNames() const
	{
		return m_Impl->fileNames;
	}

	LEOPPHAPI bool Skybox::operator==(const impl::SkyboxImpl& other) const
	{
		return m_Impl == &other;
	}

	leopph::Skybox& Skybox::operator=(const Skybox& other)
	{
		impl::DataManager::DecSkybox(m_Impl);
		m_Impl = other.m_Impl;
		impl::DataManager::IncSkybox(m_Impl);

		return *this;
	}

	leopph::Skybox& Skybox::operator=(Skybox&& other) noexcept
	{
		return operator=(other);
	}

	bool Skybox::operator==(const Skybox& other) const
	{
		return m_Impl == other.m_Impl;
	}
}