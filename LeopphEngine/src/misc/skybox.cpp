#include "Skybox.hpp"

#include "../rendering/SkyboxHandle.hpp"


namespace leopph
{
	Skybox::Skybox(const std::filesystem::path& left, const std::filesystem::path& right, const std::filesystem::path& top, const std::filesystem::path& bottom, const std::filesystem::path& back, const std::filesystem::path& front) :
		Proxy{right, left, top, bottom, front, back}
	{
	}


	Skybox::Skybox(const Skybox& other) :
		Proxy{static_cast<const Proxy&>(other)}
	{
	}


	Skybox::Skybox(Skybox&& other) noexcept :
		Skybox{ other }
	{}


	Skybox& Skybox::operator=(const Skybox& other)
	{
		Proxy::operator=(static_cast<const Proxy&>(other));
		return *this;
	}


	Skybox& Skybox::operator=(Skybox&& other) noexcept
	{
		return operator=(other);
	}


	const std::filesystem::path& Skybox::AllFilePaths() const
	{
		return data->AllFilePaths();
	}


	const std::filesystem::path& Skybox::RightPath() const
	{
		return data->RightPath();
	}


	const std::filesystem::path& Skybox::LeftPath() const
	{
		return data->LeftPath();
	}


	const std::filesystem::path& Skybox::TopPath() const
	{
		return data->TopPath();
	}


	const std::filesystem::path& Skybox::BottomPath() const
	{
		return data->BottomPath();
	}


	const std::filesystem::path& Skybox::FrontPath() const
	{
		return data->FrontPath();
	}


	const std::filesystem::path& Skybox::BackPath() const
	{
		return data->BackPath();
	}
}