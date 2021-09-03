#pragma once

#include "../DataManager.hpp"
#include "Resource.hpp"
#include "ResourceHandleBase.hpp"
#include "UniqueResource.hpp"

#include <concepts>
#include <filesystem>
#include <utility>

// TODO this leaks lots of headers


namespace leopph::impl
{
	template<typename T>
		requires std::derived_from<T, Resource> || std::derived_from<T, UniqueResource>
		class ResourceHandle : public ResourceHandleBase
	{
	public:
		explicit ResourceHandle(auto&&... args)
			requires std::derived_from<T, Resource> :
			m_ResPtr{ new T{ std::forward<decltype(args)>(args)... } }, resource{ m_ResPtr }
		{
			Init();
		}


		explicit ResourceHandle(std::filesystem::path path)
			requires std::derived_from<T, UniqueResource> :
		m_ResPtr{ static_cast<T*>(DataManager::Find(path)) }, resource{ m_ResPtr }
		{
			if (m_ResPtr == nullptr)
			{
				m_ResPtr = new T{ std::forward<std::filesystem::path>(path) };
			}

			Init();
		}


		ResourceHandle(const ResourceHandle& other) :
			m_ResPtr{ other.m_ResPtr }, resource{ m_ResPtr }
		{
			Init();
		}


		ResourceHandle(ResourceHandle&& other) noexcept :
			ResourceHandle{ other }
		{}


		ResourceHandle& operator=(const ResourceHandle& other)
		{
			if (&other != this)
			{
				Deinit();
				m_ResPtr = other.m_ResPtr;
				Init();
			}
			return *this;
		}


		ResourceHandle& operator=(ResourceHandle&& other) noexcept
		{
			return this->operator=(other);
		}


		~ResourceHandle() override
		{
			Deinit();
		}


	private:
		T* m_ResPtr;


		void Init()
		{
			DataManager::Register(m_ResPtr, this);
		}


		void Deinit()
		{
			DataManager::Unregister(m_ResPtr, this);

			if (DataManager::Count(m_ResPtr) == 0)
			{
				delete m_ResPtr;
			}
		}


	protected:
		T* const& resource;


		friend class Resource;
		friend class UniqueResource;
	};
}
