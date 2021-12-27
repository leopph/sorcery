#include "Skybox.hpp"

#include "SkyboxImpl.hpp"
#include "../data/DataManager.hpp"


namespace leopph
{
	Skybox::Skybox(const std::filesystem::path& right, const std::filesystem::path& left,
	               const std::filesystem::path& top, const std::filesystem::path& bottom,
	               const std::filesystem::path& front, const std::filesystem::path& back) :
		m_Impl{internal::DataManager::Instance().CreateOrGetSkyboxImpl(right.string() + internal::SkyboxImpl::FileSeparator + left.string() + internal::SkyboxImpl::FileSeparator + top.string() + internal::SkyboxImpl::FileSeparator + bottom.string() + internal::SkyboxImpl::FileSeparator + front.string() + internal::SkyboxImpl::FileSeparator + back.string())}
	{
		internal::DataManager::Instance().RegisterSkyboxHandle(m_Impl, this);
	}

	Skybox::Skybox(const Skybox& other) :
		m_Impl{other.m_Impl}
	{
		internal::DataManager::Instance().RegisterSkyboxHandle(m_Impl, this);
	}

	Skybox::Skybox(Skybox&& other) noexcept :
		Skybox{other}
	{}

	Skybox::~Skybox()
	{
		Deinit();
	}

	auto Skybox::operator=(const Skybox& other) -> Skybox&
	{
		Deinit();
		m_Impl = other.m_Impl;
		internal::DataManager::Instance().RegisterSkyboxHandle(m_Impl, this);
		return *this;
	}

	auto Skybox::operator=(Skybox&& other) noexcept -> Skybox&
	{
		return operator=(other);
	}

	auto Skybox::AllFilePaths() const -> const std::filesystem::path&
	{
		return m_Impl->AllFilePaths;
	}

	auto Skybox::RightPath() const -> const std::filesystem::path&
	{
		return m_Impl->RightPath;
	}

	auto Skybox::LeftPath() const -> const std::filesystem::path&
	{
		return m_Impl->LeftPath;
	}

	auto Skybox::TopPath() const -> const std::filesystem::path&
	{
		return m_Impl->TopPath;
	}

	auto Skybox::BottomPath() const -> const std::filesystem::path&
	{
		return m_Impl->BottomPath;
	}

	auto Skybox::FrontPath() const -> const std::filesystem::path&
	{
		return m_Impl->FrontPath;
	}

	auto Skybox::BackPath() const -> const std::filesystem::path&
	{
		return m_Impl->BackPath;
	}

	auto Skybox::Deinit() -> void
	{
		if (internal::DataManager::Instance().SkyboxHandleCount(m_Impl) == 1ull)
		{
			internal::DataManager::Instance().DestroySkyboxImpl(m_Impl);
		}
		else
		{
			internal::DataManager::Instance().UnregisterSkyboxHandle(m_Impl, this);
		}
	}
}
