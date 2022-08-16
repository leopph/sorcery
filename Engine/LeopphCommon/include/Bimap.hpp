#pragma once

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <format>
#include <functional>
#include <initializer_list>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include <utility>


namespace leopph
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


	template<class T1, class T2, Hasher<T1> Hash1 = std::hash<T1>, Hasher<T2> Hash2 = std::hash<T2>, EqualTester<T1> Equal1 = std::equal_to<T1>, EqualTester<T2> Equal2 = std::equal_to<T2>>
	class Bimap
	{
		public:
			Bimap() = default;;
			Bimap(std::initializer_list<std::pair<T1, T2>> initList);
			Bimap(std::initializer_list<std::pair<T2, T1>> initList);

			[[nodiscard]] bool is_empty() const;
			[[nodiscard]] std::size_t get_size() const;
			[[nodiscard]] std::size_t get_max_size() const;

			void clear();

			void insert(T1 const& o1, T2 const& o2 = T2{});
			void insert(T1&& o1, T2 const& o2 = T2{});
			void insert(T1 const& o1, T2&& o2 = T2{});
			void insert(T1&& o1, T2&& o2 = T2{});
			void insert(T2 const& o2, T1 const& o1 = T1{});
			void insert(T2&& o2, T1 const& o1 = T1{});
			void insert(T2 const& o2, T1&& o1 = T1{});
			void insert(T2&& o2, T1&& o1 = T1{});

			void erase(T1 const& o1);
			void erase(T2 const& o2);

			void swap(Bimap& other) noexcept;

			[[nodiscard]] bool contains(T1 const& o1);
			[[nodiscard]] bool contains(T2 const& o2);

			[[nodiscard]] T2 const& at(T1 const& o1) const;
			[[nodiscard]] T2& at(T1 const& o1);

			[[nodiscard]] T1 const& at(T2 const& o2) const;
			[[nodiscard]] T1& at(T2 const& o2);


		private:
			void insert_pointers(T1 const& left, T2 const& right);
			void erase_internal(T1 const& o1, T2 const& o2);

			std::unordered_set<T1, Hash1, Equal1> mT1;
			std::unordered_set<T2, Hash2, Equal2> mT2;
			std::unordered_map<T1*, T2*> mT1ToT2;
			std::unordered_map<T2*, T1*> mT2ToT1;

			constexpr static std::string_view ERR_MSG_FORMAT{"Bimap inconsistency detected: {}."};
	};



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::Bimap(std::initializer_list<std::pair<T1, T2>> initList)
	{
		for (auto const& pair : initList)
		{
			insert(std::move(pair.first), std::move(pair.second));
		}
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::Bimap(std::initializer_list<std::pair<T2, T1>> initList)
	{
		for (auto const& pair : initList)
		{
			insert(std::move(pair.first), std::move(pair.second));
		}
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	bool Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::is_empty() const
	{
		auto const t1Empty{mT1.empty()};

		#ifndef NDEBUG
		if (std::array const allEmpties{t1Empty, mT2.empty(), mT1ToT2.empty(), mT2ToT1.empty()};
			!std::equal(allEmpties))
		{
			throw std::runtime_error{std::format(ERR_MSG_FORMAT, "emptiness is undecidable")};
		}
		#endif

		return t1Empty;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	std::size_t Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::get_size() const
	{
		auto const firstSize{mT1.size()};

		#ifndef NDEBUG
		if (std::array const allSizes{firstSize, mT2.size(), mT1ToT2.size(), mT2ToT1.size()};
			!std::equal(allSizes))
		{
			throw std::runtime_error{std::format(ERR_MSG_FORMAT, "size is undecidable")};
		}
		#endif

		return firstSize;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	std::size_t Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::get_max_size() const
	{
		auto const firstMaxSize{mT1.max_size()};

		#ifndef NDEBUG
		if (std::array const allMaxSizes{firstMaxSize, mT2.max_size(), mT1ToT2.max_size(), mT2ToT1.max_size()};
			!std::equal(allMaxSizes))
		{
			throw std::runtime_error{std::format(ERR_MSG_FORMAT, "max size is undecidable")};
		}
		#endif

		return firstMaxSize;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::clear()
	{
		mT1.clear();
		mT2.clear();
		mT1ToT2.clear();
		mT2ToT1.clear();
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T1 const& o1, T2 const& o2)
	{
		auto const& ins1{*mT1.insert(o1).first};
		auto const& ins2{*mT2.insert(o2).first};
		insert_pointers(ins1, ins2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T1&& o1, T2 const& o2)
	{
		auto const& ins1{*mT1.insert(std::move(o1)).first};
		auto const& ins2{*mT2.insert(o2).first};
		insert_pointers(ins1, ins2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T1 const& o1, T2&& o2)
	{
		auto const& ins1{*mT1.insert(o1).first};
		auto const& ins2{*mT2.insert(std::move(o2)).first};
		insert_pointers(ins1, ins2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T1&& o1, T2&& o2)
	{
		auto const& ins1{*mT1.insert(std::move(o1)).first};
		auto const& ins2{*mT2.insert(std::move(o2)).first};
		insert_pointers(ins1, ins2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T2 const& o2, T1 const& o1)
	{
		insert(o1, o2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T2&& o2, T1 const& o1)
	{
		insert(o1, std::move(o2));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T2 const& o2, T1&& o1)
	{
		insert(std::move(o1), o2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert(T2&& o2, T1&& o1)
	{
		insert(std::move(o1), std::move(o2));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::erase(T1 const& o1)
	{
		auto const& o2{*mT2.find(mT1ToT2.find(const_cast<T1*>(&o1))->second)};
		erase_internal(o1, o2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::erase(T2 const& o2)
	{
		auto const& o1{*mT1.find(mT2ToT1.find(const_cast<T2*>(&o2))->second)};
		erase_internal(o1, o2);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::swap(Bimap& other) noexcept
	{
		mT1.swap(other.mT1);
		mT2.swap(other.mT2);
		mT1ToT2.swap(other.mT1ToT2);
		mT2ToT1.swap(other.mT2ToT1);
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	bool Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::contains(T1 const& o1)
	{
		auto const t1Contains{mT1.contains(o1)};

		#ifndef NDEBUG
		auto const t1ToT2It{mT1ToT2.find(const_cast<T1*>(&o1))};
		if (std::array const allContains{t1Contains, (t1ToT2It != nullptr), mT2.find(*t1ToT2It->second)};
			!std::equal(allContains))
		{
			throw std::runtime_error{std::format(ERR_MSG_FORMAT, "inclusion is undecidable")};
		}
		#endif

		return t1Contains;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	bool Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::contains(T2 const& o2)
	{
		auto const t2Contains{mT2.contains(o2)};

		#ifndef NDEBUG
		auto const t2ToT1It{mT1ToT2.find(const_cast<T2*>(&o2))};
		if (std::array const allContains{t2Contains, (t2ToT1It != nullptr), mT1.find(*t2ToT1It->second)};
			!std::equal(allContains))
		{
			throw std::runtime_error{std::format(ERR_MSG_FORMAT, "inclusion is undecidable")};
		}
		#endif

		return t2Contains;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	T2 const& Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::at(T1 const& o1) const
	{
		auto const& elem{*mT1.find(o1)};
		return *mT1ToT2.at(const_cast<T1*>(&elem));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	T2& Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::at(T1 const& o1)
	{
		return const_cast<T2&>(const_cast<Bimap const*>(this)->at(o1));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	T1 const& Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::at(T2 const& o2) const
	{
		auto const& elem{*mT2.find(o2)};
		return *mT2ToT1.at(const_cast<T2*>(&elem));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	T1& Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::at(T2 const& o2)
	{
		return const_cast<T1&>(const_cast<Bimap const*>(this)->at(o2));
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::insert_pointers(T1 const& left, T2 const& right)
	{
		auto const leftPtr{const_cast<T1*>(&left)};
		auto const rightPtr{const_cast<T2*>(&right)};
		mT1ToT2[leftPtr] = rightPtr;
		mT2ToT1[rightPtr] = leftPtr;
	}



	template<class T1, class T2, Hasher<T1> Hash1, Hasher<T2> Hash2, EqualTester<T1> Equal1, EqualTester<T2> Equal2>
	void Bimap<T1, T2, Hash1, Hash2, Equal1, Equal2>::erase_internal(T1 const& o1, T2 const& o2)
	{
		mT1ToT2.erase(const_cast<T1*>(&o1));
		mT2ToT1.erase(const_cast<T2*>(&o2));
		mT1.erase(o1);
		mT2.erase(o2);
	}
}
