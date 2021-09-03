#include "SkyboxHandle.hpp"

#include "SkyboxResource.hpp"


namespace leopph::impl
{
	SkyboxHandle::SkyboxHandle(const std::filesystem::path& right, const std::filesystem::path& left,
							   const std::filesystem::path& top, const std::filesystem::path& bottom,
							   const std::filesystem::path& front, const std::filesystem::path& back) :
		ResourceHandle{right.string() + SkyboxResource::FileSeparator + left.string() + SkyboxResource::FileSeparator + top.string() + SkyboxResource::FileSeparator + bottom.string() + SkyboxResource::FileSeparator + front.string() + SkyboxResource::FileSeparator + back.string()}
	{
	}


	SkyboxHandle::SkyboxHandle(const SkyboxHandle& other) :
		ResourceHandle{static_cast<const ResourceHandle&>(other)}
	{
	}


	SkyboxHandle::SkyboxHandle(SkyboxHandle&& other) noexcept :
		SkyboxHandle{other}
	{
	}


	SkyboxHandle& SkyboxHandle::operator=(const SkyboxHandle& other)
	{
		ResourceHandle::operator=(static_cast<const ResourceHandle&>(other));
		return *this;
	}


	SkyboxHandle& SkyboxHandle::operator=(SkyboxHandle&& other) noexcept
	{
		return operator=(other);
	}


	const std::filesystem::path& SkyboxHandle::AllFilePaths() const
	{
		return resource->AllFilePaths;
	}


	const std::filesystem::path& SkyboxHandle::RightPath() const
	{
		return resource->RightPath;
	}


	const std::filesystem::path& SkyboxHandle::LeftPath() const
	{
		return resource->LeftPath;
	}


	const std::filesystem::path& SkyboxHandle::TopPath() const
	{
		return resource->TopPath;
	}


	const std::filesystem::path& SkyboxHandle::BottomPath() const
	{
		return resource->BottomPath;
	}


	const std::filesystem::path& SkyboxHandle::FrontPath() const
	{
		return resource->FrontPath;
	}


	const std::filesystem::path& SkyboxHandle::BackPath() const
	{
		return resource->BackPath;
	}
}