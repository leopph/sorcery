#include "Skybox.hpp"

#include "DataManager.hpp"
#include "rendering/SkyboxImpl.hpp"


namespace leopph
{
	Skybox::Skybox(std::filesystem::path const& left, std::filesystem::path const& right,
	               std::filesystem::path const& top, std::filesystem::path const& bottom,
	               std::filesystem::path const& front, std::filesystem::path const& back) :
		m_Impl{internal::DataManager::Instance().CreateOrGetSkyboxImpl(internal::SkyboxImpl::BuildAllPaths(left, right, top, bottom, front, back))}
	{
		internal::DataManager::Instance().RegisterSkyboxHandle(m_Impl, this);
	}


	Skybox::Skybox(Skybox const& other) :
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


	auto Skybox::operator=(Skybox const& other) -> Skybox&
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Impl = other.m_Impl;
		internal::DataManager::Instance().RegisterSkyboxHandle(m_Impl, this);
		return *this;
	}


	auto Skybox::operator=(Skybox&& other) noexcept -> Skybox&
	{
		operator=(other);
		return *this;
	}


	auto Skybox::RightPath() const -> std::filesystem::path const&
	{
		return m_Impl->RightPath();
	}


	auto Skybox::LeftPath() const -> std::filesystem::path const&
	{
		return m_Impl->LeftPath();
	}


	auto Skybox::TopPath() const -> std::filesystem::path const&
	{
		return m_Impl->TopPath();
	}


	auto Skybox::BottomPath() const -> std::filesystem::path const&
	{
		return m_Impl->BottomPath();
	}


	auto Skybox::FrontPath() const -> std::filesystem::path const&
	{
		return m_Impl->FrontPath();
	}


	auto Skybox::BackPath() const -> std::filesystem::path const&
	{
		return m_Impl->BackPath();
	}


	auto Skybox::AllPaths() const -> std::filesystem::path const&
	{
		return m_Impl->Path();
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
