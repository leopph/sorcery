#include "Object.hpp"

#include <set>
#include <bit>
#include <iterator>


namespace leopph {
	std::set<Object*, Object::GuidObjectLess> Object::sAllObjects;


	auto Object::GuidObjectLess::operator()(Guid const& left, Guid const& right) const -> bool {
		auto const leftFirstHalf = *reinterpret_cast<u64 const*>(&left);
		auto const leftSecondHalf = *(reinterpret_cast<u64 const*>(&left) + 1);
		auto const rightFirstHalf = *reinterpret_cast<u64 const*>(&right);
		auto const rightSecondHalf = *(reinterpret_cast<u64 const*>(&right) + 1);

		if (leftFirstHalf < rightFirstHalf ||
			(leftFirstHalf == rightFirstHalf && leftSecondHalf < rightSecondHalf)) {
			return true;
		}
		return false;
	}


	auto Object::GuidObjectLess::operator()(Object const* const left, Guid const& right) const -> bool {
		return operator()(left->GetGuid(), right);
	}


	auto Object::GuidObjectLess::operator()(Guid const& left, Object const* right) const -> bool {
		return operator()(left, right->GetGuid());
	}


	auto Object::GuidObjectLess::operator()(Object const* const left, Object const* const right) const -> bool {
		return operator()(left->GetGuid(), right->GetGuid());
	}


	auto Object::FindObjectByGuid(Guid const& guid) -> Object* {
		if (auto const it = sAllObjects.find(guid); it != std::end(sAllObjects)) {
			return *it;
		}
		return nullptr;
	}


	Object::Object() {
		sAllObjects.emplace(this);
	}


	Object::~Object() {
		sAllObjects.erase(this);
	}


	auto Object::GetGuid() const -> Guid const& {
		return mGuid;
	}


	auto Object::SetGuid(Guid const& guid) -> void {
		auto extracted{ sAllObjects.extract(this) };
		mGuid = guid;
		sAllObjects.insert(std::move(extracted));
	}


	auto Object::GetName() const noexcept -> std::string_view {
		return mName;
	}


	auto Object::SetName(std::string name) noexcept -> void {
		mName = std::move(name);
	}

	auto Object::SerializeTextual(YAML::Node& node) const -> void {
		node["guid"] = GetGuid().ToString();
		node["type"] = static_cast<int>(GetSerializationType());
	}

	auto Object::DeserializeTextual(YAML::Node const& node) -> void {
		if (!node["guid"]) {
			throw std::runtime_error{ "Failed to deserialize object from text, because guid data is missing." };
		}

		if (!node["type"]) {
			throw std::runtime_error{ "Failed to deserialize object from text, because type data is missing." };
		}

		if (static_cast<Type>(node["type"].as<int>()) != GetSerializationType()) {
			throw std::runtime_error{ "Failed to deserialize object from text, because type data does not match the object type." };
		}

		SetGuid(Guid::Parse(node["guid"].as<std::string>()));
	}


	auto Object::SerializeBinary(std::vector<u8>& out) const -> void {
		auto const& guid{ GetGuid() };
		auto const type{ GetSerializationType() };

		auto const guidBytePtr{ reinterpret_cast<u8 const*>(&guid) };
		auto const typeBytePtr{ reinterpret_cast<u8 const*>(&type) };

		if constexpr (std::endian::native == std::endian::little) {
			out.insert(std::end(out), guidBytePtr, guidBytePtr + sizeof guid);
			out.insert(std::end(out), typeBytePtr, typeBytePtr + sizeof type);
		}
		else {
			out.insert(std::end(out), std::make_reverse_iterator(guidBytePtr + sizeof guid), std::make_reverse_iterator(guidBytePtr));
			out.insert(std::end(out), std::make_reverse_iterator(typeBytePtr + sizeof type), std::make_reverse_iterator(typeBytePtr));
		}
	}


	auto Object::DeserializeBinary(std::span<u8 const> const bytes) -> BinaryDeserializationResult {
		auto constexpr serializedSize{ sizeof Guid + sizeof Type };

		if (bytes.size() < serializedSize) {
			throw std::runtime_error{ "Failed to deserialize Object, because span does not contain enough bytes to read data." };
		}

		if (*reinterpret_cast<Type const*>(bytes.subspan<sizeof(Guid), sizeof(Type)>().data()) != GetSerializationType()) {
			throw std::runtime_error{ "Failed to deserialize Object, because the type data contained in the span does not match the object type." };
		}

		if constexpr (std::endian::native == std::endian::little) {
			SetGuid(*reinterpret_cast<Guid const*>(bytes.first<sizeof(Guid)>().data()));
		}
		else {
			throw std::runtime_error{ "Object deserialization is currently only supported on little-endian architectures." };
		}

		return { serializedSize };
	}

}
