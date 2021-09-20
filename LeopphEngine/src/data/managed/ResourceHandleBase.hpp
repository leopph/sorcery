#pragma once

#include "Resource.hpp"
#include "UniqueResource.hpp"

#include <filesystem>


namespace leopph::impl
{
	class ResourceHandleBase
	{
	public:
		virtual ~ResourceHandleBase();


		protected:
		ResourceHandleBase() = default;
		ResourceHandleBase(const ResourceHandleBase& other) = default;
		ResourceHandleBase(ResourceHandleBase&& other) = default;
		ResourceHandleBase& operator=(const ResourceHandleBase& other) = default;
		ResourceHandleBase& operator=(ResourceHandleBase&& other) = default;

		static UniqueResource* Find(const std::filesystem::path& path);

		void Init(const Resource* resource) const;
		void Init(const UniqueResource* uniqueResource) const;

		void Deinit(const Resource* resource) const;
		void Deinit(const UniqueResource* uniqueResource) const;
	};
}