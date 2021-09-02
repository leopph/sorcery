#pragma once


namespace leopph::impl
{
	class ResourceHandleBase
	{
	public:
		ResourceHandleBase() = default;
		ResourceHandleBase(const ResourceHandleBase& other) = default;
		ResourceHandleBase(ResourceHandleBase&& other) = default;
		ResourceHandleBase& operator=(const ResourceHandleBase& other) = default;
		ResourceHandleBase& operator=(ResourceHandleBase&& other) = default;
		virtual ~ResourceHandleBase() = default;
	};
}