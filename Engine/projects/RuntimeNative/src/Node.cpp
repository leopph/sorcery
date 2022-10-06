#include "Node.hpp"

#include <unordered_map>
#include <memory>

namespace leopph
{
	namespace
	{
		std::unordered_map<u64, std::unique_ptr<Node>> gNodes;
		u64 gNextNodeId{1};
	}

	extern "C"
	{
		__declspec(dllexport) u64 new_node()
		{
			u64 const id{gNextNodeId++};
			gNodes[id] = std::make_unique<Node>();
			return id;
		}

		__declspec (dllexport) i32 is_node_alive(u64 const id)
		{
			return gNodes[id] != nullptr;
		}

		__declspec(dllexport) void delete_node(u64 const id)
		{
			gNodes[id].reset();
		}

		__declspec(dllexport) Vector3 const* get_node_position(u64 const id)
		{
			return &gNodes[id]->position;
		}

		__declspec(dllexport) void set_node_position(u64 const id, Vector3 const* position)
		{
			gNodes[id]->position = *position;
		}
	}
}