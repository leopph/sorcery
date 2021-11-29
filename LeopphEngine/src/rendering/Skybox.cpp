#include "Skybox.hpp"

#include "SkyboxImpl.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	Skybox::Skybox(const std::filesystem::path& right, const std::filesystem::path& left,
							   const std::filesystem::path& top, const std::filesystem::path& bottom,
							   const std::filesystem::path& front, const std::filesystem::path& back) :
		m_Impl{impl::DataManager::CreateOrGetSkyboxImpl(right.string() + impl::SkyboxImpl::FileSeparator + left.string() + impl::SkyboxImpl::FileSeparator + top.string() + impl::SkyboxImpl::FileSeparator + bottom.string() + impl::SkyboxImpl::FileSeparator + front.string() + impl::SkyboxImpl::FileSeparator + back.string())}
	{
		impl::DataManager::RegisterSkyboxHandle(m_Impl, this);
	}


	Skybox::Skybox(const Skybox& other) :
		m_Impl{other.m_Impl}
	{
		impl::DataManager::RegisterSkyboxHandle(m_Impl, this);
	}

	Skybox::Skybox(Skybox&& other) noexcept :
		Skybox{other}
	{}

	Skybox::~Skybox()
	{
		Deinit();
	}

	Skybox& Skybox::operator=(const Skybox& other)
	{
		Deinit();
		m_Impl = other.m_Impl;
		impl::DataManager::RegisterSkyboxHandle(m_Impl, this);
		return *this;
	}

	Skybox& Skybox::operator=(Skybox&& other) noexcept
	{
		return operator=(other);
	}

	const std::filesystem::path& Skybox::AllFilePaths() const
	{
		return m_Impl->AllFilePaths;
	}

	const std::filesystem::path& Skybox::RightPath() const
	{
		return m_Impl->RightPath;
	}

	const std::filesystem::path& Skybox::LeftPath() const
	{
		return m_Impl->LeftPath;
	}

	const std::filesystem::path& Skybox::TopPath() const
	{
		return m_Impl->TopPath;
	}

	const std::filesystem::path& Skybox::BottomPath() const
	{
		return m_Impl->BottomPath;
	}

	const std::filesystem::path& Skybox::FrontPath() const
	{
		return m_Impl->FrontPath;
	}

	const std::filesystem::path& Skybox::BackPath() const
	{
		return m_Impl->BackPath;
	}

	void Skybox::Deinit()
	{
		if (impl::DataManager::Skyboxes().at(*m_Impl).size() == 1ull)
		{
			impl::DataManager::DestroySkyboxImpl(m_Impl);
		}
		else
		{
			impl::DataManager::UnregisterSkyboxHandle(m_Impl, this);
		}
	}
}