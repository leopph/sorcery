#include "Skybox.hpp"

#include "InternalContext.hpp"
#include "rendering/SkyboxImpl.hpp"
#include "rendering/renderers/GlRenderer.hpp"


namespace leopph
{
	Skybox::Skybox(std::filesystem::path const& left, std::filesystem::path const& right,
	               std::filesystem::path const& top, std::filesystem::path const& bottom,
	               std::filesystem::path const& front, std::filesystem::path const& back) :
		m_Impl{static_cast<internal::GlRenderer*>(internal::GetRenderer())->CreateOrGetSkyboxImpl(internal::SkyboxImpl::BuildAllPaths(left, right, top, bottom, front, back))}
	{
		m_Impl->RegisterHandle(this);
	}


	Skybox::Skybox(Skybox const& other) :
		m_Impl{other.m_Impl}
	{
		m_Impl->RegisterHandle(this);
	}


	auto Skybox::operator=(Skybox const& other) -> Skybox&
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Impl = other.m_Impl;
		m_Impl->RegisterHandle(this);

		return *this;
	}


	Skybox::Skybox(Skybox&& other) noexcept :
		m_Impl{other.m_Impl}
	{
		m_Impl->RegisterHandle(this);
	}


	auto Skybox::operator=(Skybox&& other) noexcept -> Skybox&
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Impl = other.m_Impl;
		m_Impl->RegisterHandle(this);

		return *this;
	}


	Skybox::~Skybox()
	{
		Deinit();
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
		return m_Impl->AllPaths();
	}


	auto Skybox::Deinit() const -> void
	{
		m_Impl->UnregisterHandle(this);

		// If we were the last handle referring the impl, it is our job to delete it.
		if (m_Impl->NumHandles() == 0)
		{
			static_cast<internal::GlRenderer*>(internal::GetRenderer())->DestroySkyboxImpl(m_Impl);
		}
	}
}
