#include "SkyboxHandle.hpp"

#include "SkyboxImpl.hpp"
#include "../data/DataManager.hpp"


namespace leopph::impl
{
	SkyboxHandle::SkyboxHandle(const std::filesystem::path& right, const std::filesystem::path& left,
							   const std::filesystem::path& top, const std::filesystem::path& bottom,
							   const std::filesystem::path& front, const std::filesystem::path& back) :
		m_Impl{DataManager::CreateOrGetSkyboxImpl(right.string() + SkyboxImpl::FileSeparator + left.string() + SkyboxImpl::FileSeparator + top.string() + SkyboxImpl::FileSeparator + bottom.string() + SkyboxImpl::FileSeparator + front.string() + SkyboxImpl::FileSeparator + back.string())}
	{
		DataManager::RegisterSkyboxHandle(m_Impl, this);
	}


	SkyboxHandle::SkyboxHandle(const SkyboxHandle& other) :
		m_Impl{other.m_Impl}
	{
		DataManager::RegisterSkyboxHandle(m_Impl, this);
	}

	SkyboxHandle::SkyboxHandle(SkyboxHandle&& other) noexcept :
		SkyboxHandle{other}
	{}

	SkyboxHandle::~SkyboxHandle()
	{
		Deinit();
	}

	SkyboxHandle& SkyboxHandle::operator=(const SkyboxHandle& other)
	{
		Deinit();
		m_Impl = other.m_Impl;
		DataManager::RegisterSkyboxHandle(m_Impl, this);
		return *this;
	}

	SkyboxHandle& SkyboxHandle::operator=(SkyboxHandle&& other) noexcept
	{
		return operator=(other);
	}

	const std::filesystem::path& SkyboxHandle::AllFilePaths() const
	{
		return m_Impl->AllFilePaths;
	}

	const std::filesystem::path& SkyboxHandle::RightPath() const
	{
		return m_Impl->RightPath;
	}

	const std::filesystem::path& SkyboxHandle::LeftPath() const
	{
		return m_Impl->LeftPath;
	}

	const std::filesystem::path& SkyboxHandle::TopPath() const
	{
		return m_Impl->TopPath;
	}

	const std::filesystem::path& SkyboxHandle::BottomPath() const
	{
		return m_Impl->BottomPath;
	}

	const std::filesystem::path& SkyboxHandle::FrontPath() const
	{
		return m_Impl->FrontPath;
	}

	const std::filesystem::path& SkyboxHandle::BackPath() const
	{
		return m_Impl->BackPath;
	}

	void SkyboxHandle::Deinit()
	{
		if (DataManager::Skyboxes().at(*m_Impl).size() == 1ull)
		{
			DataManager::DestroySkyboxImpl(m_Impl);
		}
		else
		{
			DataManager::UnregisterSkyboxHandle(m_Impl, this);
		}
	}
}