#include "Skybox.hpp"

#include "InternalContext.hpp"
#include "rendering/gl/GlSkyboxImpl.hpp"
#include "rendering/renderers/GlRenderer.hpp"


namespace leopph
{
	Skybox::Skybox(std::filesystem::path const& left, std::filesystem::path const& right,
	               std::filesystem::path const& top, std::filesystem::path const& bottom,
	               std::filesystem::path const& front, std::filesystem::path const& back) :
		m_Impl{static_cast<internal::GlRenderer*>(internal::GetRenderer())->CreateOrGetSkyboxImpl(internal::GlSkyboxImpl::build_all_paths(left, right, top, bottom, front, back))}
	{
		m_Impl->register_handle(this);
	}


	Skybox::Skybox(Skybox const& other) :
		m_Impl{other.m_Impl}
	{
		m_Impl->register_handle(this);
	}


	Skybox& Skybox::operator=(Skybox const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Impl = other.m_Impl;
		m_Impl->register_handle(this);

		return *this;
	}


	Skybox::Skybox(Skybox&& other) noexcept :
		m_Impl{other.m_Impl}
	{
		m_Impl->register_handle(this);
	}


	Skybox& Skybox::operator=(Skybox&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		Deinit();
		m_Impl = other.m_Impl;
		m_Impl->register_handle(this);

		return *this;
	}


	Skybox::~Skybox()
	{
		Deinit();
	}


	std::filesystem::path const& Skybox::RightPath() const
	{
		return m_Impl->right_path();
	}


	std::filesystem::path const& Skybox::LeftPath() const
	{
		return m_Impl->left_path();
	}


	std::filesystem::path const& Skybox::TopPath() const
	{
		return m_Impl->top_path();
	}


	std::filesystem::path const& Skybox::BottomPath() const
	{
		return m_Impl->bottom_path();
	}


	std::filesystem::path const& Skybox::FrontPath() const
	{
		return m_Impl->front_path();
	}


	std::filesystem::path const& Skybox::BackPath() const
	{
		return m_Impl->back_path();
	}


	std::filesystem::path const& Skybox::AllPaths() const
	{
		return m_Impl->AllPaths();
	}


	void Skybox::Deinit() const
	{
		m_Impl->unregister_handle(this);

		// If we were the last handle referring the impl, it is our job to delete it.
		if (m_Impl->num_handles() == 0)
		{
			static_cast<internal::GlRenderer*>(internal::GetRenderer())->DestroySkyboxImpl(m_Impl);
		}
	}
}
