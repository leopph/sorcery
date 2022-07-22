#pragma once

#include <cstddef>
#include <utility>
#include <vector>


namespace leopph::internal
{
	// Implementation of a FIFO data structure using contiguous memory.
	template<class T>
	class ArrayQueue
	{
		public:
			// Returns a reference to the next element about to be popped.
			// Calling on an empty ArrayQueue results in undefined behavior.
			[[nodiscard]] const T& Peek() const;

			// Add a new element to the queue's end.
			void Push(T val);

			// Removes the next element from the queue and returns it.
			// Calling on an empty ArrayQueue results in undefined behavior.
			T Pop();

			// Returns whether the ArrayQueue contains any elements.
			[[nodiscard]] bool Empty() const;

		private:
			std::vector<T> m_Vec;
			std::size_t m_Head{0};
			std::size_t m_Tail{0};
	};


	template<class T>
	const T& ArrayQueue<T>::Peek() const
	{
		return m_Vec[m_Head];
	}


	template<class T>
	void ArrayQueue<T>::Push(T val)
	{
		if (m_Vec.empty())
		{
			m_Vec.emplace_back(std::move(val));
			m_Head = 0;
			m_Tail = 1;
			return;
		}

		if (m_Head < m_Tail)
		{
			// If head is at the front or there is free space left in the vector
			// Append to end
			if (m_Head == 0 || m_Tail < m_Vec.capacity())
			{
				m_Vec.emplace_back(std::move(val));
				++m_Tail;
				return;
			}

			// Otherwise overflow to the front of the buffer
			m_Tail = 1;
			m_Vec[0] = std::move(val);
			return;
		}

		if (m_Head > m_Tail)
		{
			m_Vec[m_Tail] = std::move(val);
			++m_Tail;
			return;
		}

		// head == tail, so buffer is full
		// Move elements and reallocate

		// The number of elements spanning from head to the end of the buffer
		auto const numHeadElems{m_Vec.capacity() - m_Head};
		for (auto i = 0ull; i < numHeadElems; ++i)
		{
			// The currently moved element's index
			auto const curMovId{m_Head + i};
			auto movElem{std::move(m_Vec[curMovId])};
			for (auto j = curMovId; j > i; --j)
			{
				m_Vec[j] = std::move(m_Vec[j - 1]);
			}
			m_Vec[i] = std::move(movElem);
		}

		m_Head = 0;
		m_Tail = m_Vec.capacity();

		m_Vec.emplace_back(std::move(val));
		++m_Tail;
	}


	template<class T>
	T ArrayQueue<T>::Pop()
	{
		T ret{std::move(m_Vec[m_Head])};

		if (m_Head < m_Tail)
		{
			++m_Head;
		}
		else
		{
			m_Head = (m_Head + 1) % m_Vec.capacity();
		}

		if (m_Head == m_Tail)
		{
			m_Vec.clear();
		}

		return ret;
	}


	template<class T>
	bool ArrayQueue<T>::Empty() const
	{
		return m_Head == m_Tail;
	}
}
