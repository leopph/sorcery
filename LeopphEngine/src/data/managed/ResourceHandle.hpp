#pragma once

#include "Resource.hpp"
#include "ResourceHandleBase.hpp"
#include "UniqueResource.hpp"

#include <concepts>
#include <filesystem>
#include <utility>



namespace leopph::impl
{
	template<typename T>
		requires std::derived_from<T, Resource> || std::derived_from<T, UniqueResource>
	class ResourceHandle : public ResourceHandleBase
	{
		public:
			explicit ResourceHandle(auto&&... args)
				requires std::derived_from<T, Resource> :
				m_ResPtr{new T{std::forward<decltype(args)>(args)...}},
				resource{m_ResPtr}
			{
				Init(m_ResPtr);
			}


			explicit ResourceHandle(std::filesystem::path path)
				requires std::derived_from<T, UniqueResource> :
				m_ResPtr{static_cast<T*>(Find(path))},
				resource{m_ResPtr}
			{
				if (m_ResPtr == nullptr)
				{
					m_ResPtr = new T{std::forward<std::filesystem::path>(path)};
				}

				Init(m_ResPtr);
			}


			ResourceHandle(const ResourceHandle& other) :
				m_ResPtr{other.m_ResPtr},
				resource{m_ResPtr}
			{
				Init(m_ResPtr);
			}


			ResourceHandle(ResourceHandle&& other) noexcept :
				ResourceHandle{other}
			{}


			ResourceHandle& operator=(const ResourceHandle& other)
			{
				if (&other != this)
				{
					Deinit(m_ResPtr);
					m_ResPtr = other.m_ResPtr;
					Init(m_ResPtr);
				}
				return *this;
			}


			ResourceHandle& operator=(ResourceHandle&& other) noexcept
			{
				return this->operator=(other);
			}


			~ResourceHandle() override
			{
				Deinit(m_ResPtr);
			}


		private:
			T* m_ResPtr;


		protected:
			T* const& resource;


			friend class Resource;
			friend class UniqueResource;
	};
}
