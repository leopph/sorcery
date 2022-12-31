#pragma once

#include "YamlInclude.hpp"
#include "Math.hpp"


namespace YAML {
	template<typename T, std::size_t N>
	struct convert<leopph::Vector<T, N>> {
		static auto encode(leopph::Vector<T, N> const& v) -> Node {
			Node node;
			node.SetStyle(YAML::EmitterStyle::Flow);
			for (std::size_t i = 0; i < N; i++) {
				node.push_back(v[i]);
			}
			return node;
		}

		static auto decode(Node const& node, leopph::Vector<T, N>& v) -> bool {
			if (!node.IsSequence() || node.size() != N) {
				return false;
			}
			for (std::size_t i = 0; i < N; i++) {
				v[i] = node[i].as<T>();
			}
			return true;
		}
	};

	template<>
	struct convert<leopph::Quaternion> {
		static auto encode(leopph::Quaternion const& q) -> Node {
			Node node;
			node.SetStyle(YAML::EmitterStyle::Flow);
			node.push_back(q.w);
			node.push_back(q.x);
			node.push_back(q.y);
			node.push_back(q.z);
			return node;
		}

		static auto decode(Node const& node, leopph::Quaternion& q) -> bool {
			if (!node.IsSequence() || node.size() != 4) {
				return false;
			}
			q.w = node[0].as<leopph::f32>();
			q.x = node[1].as<leopph::f32>();
			q.y = node[2].as<leopph::f32>();
			q.z = node[3].as<leopph::f32>();
			return true;
		}
	};
}