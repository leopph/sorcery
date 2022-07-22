#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace leopph::internal
{
	template<class Hash, class T>
	concept Hasher = requires(Hash const& hasher, T hashed)
	{
		{
			hasher(hashed)
		} -> std::same_as<std::size_t>;
	};

	template<class Equal, class T>
	concept EqualTester =
	requires(Equal const& equal, T const& left, T const& right)
	{
		{
			equal(left, right)
		} -> std::same_as<bool>;
	};


	template<class T1, class T2, bool SafeMode = false, Hasher<T1> Hash1 = std::hash<T1>, Hasher<T2> Hash2 = std::hash<T2>, EqualTester<T1> Equal1 = std::equal_to<T1>, EqualTester<T2> Equal2 = std::equal_to<T2>>
	class Bimap
	{
		public:
			Bimap() = default;

			Bimap(Bimap const& other) = default;


			Bimap(Bimap&& other) noexcept :
				m_T1ToT2{std::move(other.m_T1ToT2)},
				m_T2ToT1{std::move(other.m_T2ToT1)}
			{}


			Bimap(std::initializer_list<std::pair<T1, T2>> initList)
			{
				std::ranges::for_each(initList, [this](auto& pair)
				{
					Insert(std::move(pair.first), std::move(pair.second));
				});
			}


			Bimap(std::initializer_list<std::pair<T2, T1>> initList)
			{
				std::ranges::for_each(initList, [this](auto& pair)
				{
					Insert(std::move(pair.first), std::move(pair.second));
				});
			}


			Bimap& operator=(Bimap const& other) = default;


			Bimap& operator=(Bimap&& other) noexcept
			{
				m_T1ToT2 = std::move(other.m_T1ToT2);
				m_T2ToT1 = std::move(other.m_T1ToT2);
				return *this;
			}


			~Bimap() = default;


			[[nodiscard]] auto Empty() const
			{
				auto const t1Empty{m_T1.empty()};

				if constexpr (SafeMode)
				{
					if (std::array const allEmpties{t1Empty, m_T2.empty(), m_T1ToT2.empty(), m_T2ToT1.empty()};
						!std::equal(allEmpties))
					{
						throw std::runtime_error{ERR_MSG_PREFIX + "emptiness is undecidable."};
					}
				}

				return t1Empty;
			}


			[[nodiscard]] auto Size() const
			{
				auto const firstSize{m_T1.size()};

				if constexpr (SafeMode)
				{
					if (std::array const allSizes{firstSize, m_T2.size(), m_T1ToT2.size(), m_T2ToT1.size()};
						!std::equal(allSizes))
					{
						throw std::runtime_error{ERR_MSG_PREFIX + "size is undecidable."};
					}
				}

				return firstSize;
			}


			[[nodiscard]] auto MaxSize() const
			{
				auto const firstMaxSize{m_T1.max_size()};

				if constexpr (SafeMode)
				{
					if (std::array const allMaxSizes{firstMaxSize, m_T2.max_size(), m_T1ToT2.max_size(), m_T2ToT1.max_size()};
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


			auto Insert(T1 const& o1, T2 const& o2 = T2{})
			{
				auto const& ins1{*m_T1.insert(o1).first};
				auto const& ins2{*m_T2.insert(o2).first};
				InsertPointers(ins1, ins2);
			}


			auto Insert(T1&& o1, T2 const& o2 = T2{})
			{
				auto const& ins1{*m_T1.insert(std::move(o1)).first};
				auto const& ins2{*m_T2.insert(o2).first};
				InsertPointers(ins1, ins2);
			}


			auto Insert(T1 const& o1, T2&& o2 = T2{})
			{
				auto const& ins1{*m_T1.insert(o1).first};
				auto const& ins2{*m_T2.insert(std::move(o2)).first};
				InsertPointers(ins1, ins2);
			}


			auto Insert(T1&& o1, T2&& o2 = T2{})
			{
				auto const& ins1{*m_T1.insert(std::move(o1)).first};
				auto const& ins2{*m_T2.insert(std::move(o2)).first};
				InsertPointers(ins1, ins2);
			}


			auto Insert(T2 const& o2, T1 const& o1 = T1{})
			{
				Insert(o1, o2);
			}


			auto Insert(T2&& o2, T1 const& o1 = T1{})
			{
				Insert(o1, std::move(o2));
			}


			auto Insert(T2 const& o2, T1&& o1 = T1{})
			{
				Insert(std::move(o1), o2);
			}


			auto Insert(T2&& o2, T1&& o1 = T1{})
			{
				Insert(std::move(o1), std::move(o2));
			}


			auto Erase(T1 const& o1)
			{
				auto const& o2{*m_T2.find(m_T1ToT2.find(const_cast<T1*>(&o1))->second)};
				Erase(o1, o2);
			}


			auto Erase(T2 const& o2)
			{
				auto const& o1{*m_T1.find(m_T2ToT1.find(const_cast<T2*>(&o2))->second)};
				Erase(o1, o2);
			}


			auto Swap(Bimap& other)
			{
				m_T1.swap(other.m_T1);
				m_T2.swap(other.m_T2);
				m_T1ToT2.swap(other.m_T1ToT2);
				m_T2ToT1.swap(other.m_T2ToT1);
			}


			[[nodiscard]] auto Contains(T1 const& o1)
			{
				auto const t1Contains{m_T1.contains(o1)};

				if constexpr (SafeMode)
				{
					auto const t1ToT2It{m_T1ToT2.find(const_cast<T1*>(&o1))};
					if (std::array const allContains{t1Contains, (t1ToT2It != nullptr), m_T2.find(*t1ToT2It->second)};
						!std::equal(allContains))
					{
						throw std::runtime_error{ERR_MSG_PREFIX + "inclusion is undecidable."};
					}
				}

				return t1Contains;
			}


			[[nodiscard]] auto Contains(T2 const& o2)
			{
				auto const t2Contains{m_T2.contains(o2)};

				if constexpr (SafeMode)
				{
					auto const t2ToT1It{m_T1ToT2.find(const_cast<T2*>(&o2))};
					if (std::array const allContains{t2Contains, (t2ToT1It != nullptr), m_T1.find(*t2ToT1It->second)};
						!std::equal(allContains))
					{
						throw std::runtime_error{ERR_MSG_PREFIX + "inclusion is undecidable."};
					}
				}

				return t2Contains;
			}


			[[nodiscard]] auto const& operator[](T1 const& o1) const
			{
				auto const& elem{*m_T1.find(o1)};
				return m_T1ToT2[const_cast<T1*>(&elem)];
			}


			[[nodiscard]] auto& operator[](T1 const& o1)
			{
				return const_cast<T2&>(const_cast<Bimap const*>(this)->operator[](o1));
			}


			[[nodiscard]] auto const& operator[](T2 const& o2) const
			{
				auto const& elem{*m_T2.find(o2)};
				return m_T2ToT1[const_cast<T2*>(&elem)];
			}


			[[nodiscard]] auto& operator[](T2 const& o2)
			{
				return const_cast<T1&>(const_cast<Bimap const*>(this)->operator[](o2));
			}


			[[nodiscard]] auto const& At(T1 const& o1) const
			{
				auto const& elem{*m_T1.find(o1)};
				return *m_T1ToT2.at(const_cast<T1*>(&elem));
			}


			[[nodiscard]] auto& At(T1 const& o1)
			{
				return const_cast<T2&>(const_cast<Bimap const*>(this)->At(o1));
			}


			[[nodiscard]] auto const& At(T2 const& o2) const
			{
				auto const& elem{*m_T2.find(o2)};
				return *m_T2ToT1.at(const_cast<T2*>(&elem));
			}


			[[nodiscard]] auto& At(T2 const& o2)
			{
				return const_cast<T1&>(const_cast<Bimap const*>(this)->At(o2));
			}


		private:
			void InsertPointers(T1 const& left, T2 const& right)
			{
				auto const leftPtr{const_cast<T1*>(&left)};
				auto const rightPtr{const_cast<T2*>(&right)};
				m_T1ToT2[leftPtr] = rightPtr;
				m_T2ToT1[rightPtr] = leftPtr;
			}


			void Erase(T1 const& o1, T2 const& o2)
			{
				m_T1ToT2.erase(const_cast<T1*>(&o1));
				m_T2ToT1.erase(const_cast<T2*>(&o2));
				m_T1.erase(o1);
				m_T2.erase(o2);
			}


			constexpr static std::string ERR_MSG_PREFIX{"Bimap inconsistency detected : "};

			std::unordered_set<T1, Hash1, Equal1> m_T1;
			std::unordered_set<T2, Hash2, Equal2> m_T2;
			std::unordered_map<T1*, T2*> m_T1ToT2;
			std::unordered_map<T2*, T1*> m_T2ToT1;
	};
}
