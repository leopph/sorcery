#pragma once

#include "Types.hpp"

#include <cstddef>


namespace leopph
{
	class PersistentMappedBuffer
	{
		public:
			explicit PersistentMappedBuffer(u64 size);

			PersistentMappedBuffer(PersistentMappedBuffer const& other) = delete;
			PersistentMappedBuffer(PersistentMappedBuffer&& other) = delete;

			PersistentMappedBuffer& operator=(PersistentMappedBuffer const& other) = delete;
			PersistentMappedBuffer& operator=(PersistentMappedBuffer&& other) = delete;

			~PersistentMappedBuffer();

			template<class T>
			T& at(std::size_t index);

			template<class T>
			T const& at(std::size_t index);

			[[nodiscard]] void* get_ptr() const;
			[[nodiscard]] u32 get_internal_handle() const;
			[[nodiscard]] u64 get_size() const;


		private:
			u32 mName{};
			u64 mSize;
			void* mMapping{};
	};



	template<class T>
	T& PersistentMappedBuffer::at(std::size_t const index)
	{
		return static_cast<T*>(mMapping)[index];
	}



	template<class T>
	T const& PersistentMappedBuffer::at(std::size_t index)
	{
		return static_cast<T const*>(mMapping)[index];
	}
}
