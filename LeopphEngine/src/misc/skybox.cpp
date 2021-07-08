#include "skybox.h"

#include "../instances/instanceholder.h"

#include <stdexcept>

namespace leopph
{
	Skybox::Skybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front) :
		m_Impl{ impl::InstanceHolder::GetSkybox(left, right, top, bottom, back, front) }
	{
		if (m_Impl == nullptr)
			m_Impl = impl::InstanceHolder::RegisterSkybox(left, right, top, bottom, back, front);
		else
			impl::InstanceHolder::IncSkybox(m_Impl);
	}

	Skybox::Skybox(const Skybox& other) :
		m_Impl{ other.m_Impl }
	{
		impl::InstanceHolder::IncSkybox(m_Impl);
	}

	Skybox::Skybox(Skybox&& other) noexcept :
		Skybox{ other }
	{}

	Skybox::~Skybox()
	{
		impl::InstanceHolder::DecSkybox(m_Impl);
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
		impl::InstanceHolder::DecSkybox(m_Impl);
		m_Impl = other.m_Impl;
		impl::InstanceHolder::IncSkybox(m_Impl);

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