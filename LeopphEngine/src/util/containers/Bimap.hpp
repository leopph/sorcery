#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace leopph::impl
{
	template<class Hasher, class T1, class T2>
	concept Hash =
	requires(const Hasher& hasher, T1 hashed)
	{
		{
			hasher(hashed)
		} -> std::same_as<std::size_t>;
	}
	&&
	requires(const Hasher& hasher, T2 hashed)
	{
		{
			hasher(hashed)
		} -> std::same_as<std::size_t>;
	};


	template<class EqualTester, class T1, class T2>
	concept Equal =
	requires(const EqualTester& tester, T1 tested1, T1 tested2)
	{
		{
			tester(tested1, tested2)
		} -> std::same_as<bool>;
	}
	&&
	requires(const EqualTester& tester, T2 tested1, T2 tested2)
	{
		{
			tester(tested1, tested2)
		} -> std::same_as<bool>;
	};



	template<class T1, class T2, Hash<T1, T2> Hash, Equal<T1, T2> Equal, bool SafeMode = false>
	class Bimap
	{
		public:
			Bimap() = default;


			Bimap(const Bimap& other) = default;


			Bimap(Bimap&& other) noexcept :
				m_T1ToT2{std::move(other.m_T1ToT2)},
				m_T2ToT1{std::move(other.m_T2ToT1)}
			{}


			Bimap& operator=(const Bimap& other) = default;


			Bimap& operator=(Bimap&& other) noexcept
			{
				m_T1ToT2 = std::move(other.m_T1ToT2);
				m_T2ToT1 = std::move(other.m_T1ToT2);
				return *this;
			}


			~Bimap() = default;


		[[nodiscard]] auto Empty() const
		{
			const auto t1Empty{m_T1.empty()};

			if constexpr (SafeMode)
			{
				if (const std::array allEmpties{t1Empty, m_T2.empty(), m_T1ToT2.empty(), m_T2ToT1.empty()};
					!std::equal(allEmpties))
				{
						throw std::runtime_error{ERR_MSG_PREFIX + "emptiness is undecidable."};
				}
			}

			return t1Empty;
		}


		[[nodiscard]] auto Size() const
		{
			const auto firstSize{m_T1.size()};

			if constexpr (SafeMode)
			{
				if (const std::array allSizes{firstSize, m_T2.size(), m_T1ToT2.size(), m_T2ToT1.size()};
					!std::equal(allSizes))
				{
					throw std::runtime_error{ERR_MSG_PREFIX + "size is undecidable."};
				}
			}

			return firstSize;
		}


		[[nodiscard]] auto MaxSize() const
		{
			const auto firstMaxSize{m_T1.max_size()};

			if constexpr (SafeMode)
			{
				if (const std::array allMaxSizes{firstMaxSize, m_T2.max_size(), m_T1ToT2.max_size(), m_T2ToT1.max_size()};
					!std::equal(allMaxSizes))
				{
					throw std::runtime_error{ERR_MSG_PREFIX + "max size is undecidable."};
				}
			}

			return firstMaxSize;
		}


		auto Clear()
		{
			m_T1.clear();
			m_T2.clear();
			m_T1ToT2.clear();
			m_T2ToT1.clear();
		}


		auto Insert(const T1& o1, const T2& o2 = T2{})
		{
			const auto& ins1{*m_T1.insert(o1).first};
			const auto& ins2{*m_T2.insert(o2).first};
			InsertPointers(ins1, ins2);
		}


		auto Insert(T1&& o1, const T2& o2 = T2{})
		{
			const auto& ins1{*m_T1.insert(std::move(o1)).first};
			const auto& ins2{*m_T2.insert(o2).first};
			InsertPointers(ins1, ins2);
		}


		auto Insert(const T1& o1, T2&& o2 = T2{})
		{
			const auto& ins1{*m_T1.insert(o1).first};
			const auto& ins2{*m_T2.insert(std::move(o2)).first};
			InsertPointers(ins1, ins2);
		}


		auto Insert(T1&& o1, T2&& o2 = T2{})
		{
			const auto& ins1{*m_T1.insert(std::move(o1)).first};
			const auto& ins2{*m_T2.insert(std::move(o2))};
			InsertPointers(ins1, ins2);
		}


		auto Insert(const T2& o2, const T1& o1 = T1{})
		{
			Insert(o1, o2);
		}


		auto Insert(T2&& o2, const T1& o1 = T1{})
		{
			Insert(o1, std::move(o2));
		}


		auto Insert(const T2& o2, T1&& o1 = T1{})
		{
			Insert(std::move(o1), o2);
		}


		auto Insert(T2&& o2, T1&& o1 = T1{})
		{
			Insert(std::move(o1), std::move(o2));
		}


		auto Erase(const T1& o1)
		{
			const auto& o2{*m_T2.find(m_T1ToT2.find(const_cast<T1*>(&o1))->second)};
			Erase(o1, o2);
		}


		auto Erase(const T2& o2)
		{
			const auto& o1{*m_T1.find(m_T2ToT1.find(const_cast<T2*>(&o2))->second)};
			Erase(o1, o2);
		}


		auto Swap(Bimap& other)
		{
			m_T1.swap(other.m_T1);
			m_T2.swap(other.m_T2);
			m_T1ToT2.swap(other.m_T1ToT2);
			m_T2ToT1.swap(other.m_T2ToT1);
		}


		[[nodiscard]] auto Contains(const T1& o1)
		{
			const auto t1Contains{m_T1.contains(o1)};

			if constexpr (SafeMode)
			{
				const auto t1ToT2It{m_T1ToT2.find(const_cast<T1*>(&o1))};
				if (const std::array allContains{t1Contains, (t1ToT2It != nullptr), m_T2.find(*t1ToT2It->second)};
					!std::equal(allContains))
				{
					throw std::runtime_error{ERR_MSG_PREFIX + "inclusion is undecidable."};
				}
			}

			return t1Contains;
		}


		[[nodiscard]] auto Contains(const T2& o2)
		{
			const auto t2Contains{m_T2.contains(o2)};

			if constexpr (SafeMode)
			{
				const auto t2ToT1It{m_T1ToT2.find(const_cast<T2*>(&o2))};
				if (const std::array allContains{t2Contains, (t2ToT1It != nullptr), m_T1.find(*t2ToT1It->second)};
					!std::equal(allContains))
				{
					throw std::runtime_error{ERR_MSG_PREFIX + "inclusion is undecidable."};
				}
			}

			return t2Contains;
		}


		const auto& operator[](const T1& o1) const
		{
			return m_T1ToT2[const_cast<T1*>(&o1)];
		}


		auto& operator[](const T1& o1)
		{
			return const_cast<T2&>(const_cast<const Bimap*>(this)->operator[](o1));
		}


		const auto& operator[](const T2& o2) const
		{
			return m_T2ToT1[const_cast<T2*>(&o2)];
		}


		auto& operator[](const T2& o2)
		{
			return const_cast<T1&>(const_cast<const Bimap*>(this)->operator[](o2));
		}


		const auto& At(const T1& o1) const
		{
			return *m_T1ToT2.at(const_cast<T1*>(&o1));
		}


		auto& At(const T1& o1)
		{
			return const_cast<T2&>(const_cast<const Bimap*>(this)->At(o1));
		}


		const auto& At(const T2& o2) const
		{
			return *m_T2ToT1.at(const_cast<T2*>(&o2));
		}

		auto& At(const T2& o2)
		{
			return const_cast<T1&>(const_cast<const Bimap*>(this)->At(o2));
		}


		private:
			void InsertPointers(const T1& left, const T2& right)
			{
				const auto leftPtr{const_cast<T1*>(&left)};
				const auto rightPtr{const_cast<T2*>(&right)};
				m_T1ToT2.insert(leftPtr, rightPtr);
				m_T2ToT1.insert(rightPtr, leftPtr);
			}

			void Erase(const T1& o1, const T2& o2)
			{
				m_T1ToT2.erase(const_cast<T1*>(&o1));
				m_T2ToT1.erase(const_cast<T2*>(&o2));
				m_T1.erase(o1);
				m_T2.erase(o2);
			}

			constexpr static std::string ERR_MSG_PREFIX{"Bimap inconsistency detected : "};

			std::unordered_set<T1, Hash, Equal> m_T1;
			std::unordered_set<T2, Hash, Equal> m_T2;
			std::unordered_map<T1*, T2*> m_T1ToT2;
			std::unordered_map<T2*, T1*> m_T2ToT1;
	};
}
