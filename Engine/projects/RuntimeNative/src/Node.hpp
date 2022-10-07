#pragma once

#include "Math.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <span>
#include <vector>


namespace leopph
{
	/*struct Node
	{
		u64 id{};
		std::wstring name{};
		Vector3 position{0, 0, 0};
		Quaternion rotation{0, 0, 0, 1};
		Vector3 scale{1, 1, 1};
	};*/

	enum class Space : u8
	{
		World = 0,
		Object = 1
	};


	class Node
	{
	private:
		static u64 sNextId;

		u64 mId{sNextId++};
		//Scene* mScene;
		std::string mName{"Node"};

		Vector3 mLocalPosition{0};
		Quaternion mLocalRotation{1, 0, 0, 0};
		Vector3 mLocalScale{1};

		Vector3 mWorldPosition{mLocalPosition};
		Quaternion mWorldRotation{mLocalRotation};
		Vector3 mWorldScale{mLocalScale};

		Vector3 mForward{Vector3::forward()};
		Vector3 mRight{Vector3::right()};
		Vector3 mUp{Vector3::up()};

		Node* mParent{nullptr};
		std::vector<Node*> mChildren;

		Matrix4 mModelMat{Matrix4::identity()};
		Matrix3 mNormalMat{Matrix4::identity()};

	public:
		LEOPPHAPI Node();
		LEOPPHAPI Node(Node const& other);
		LEOPPHAPI Node(Node&& other) noexcept;

		LEOPPHAPI Node& operator=(Node const& other);
		LEOPPHAPI Node& operator=(Node&& other) noexcept;

		virtual LEOPPHAPI ~Node();

		[[nodiscard]] LEOPPHAPI std::string_view get_name() const;
		LEOPPHAPI void set_name(std::string name);

		[[nodiscard]] LEOPPHAPI Vector3 const& get_position() const;
		LEOPPHAPI void set_position(Vector3 const& newPos);

		[[nodiscard]] LEOPPHAPI Vector3 const& get_local_position() const;
		LEOPPHAPI void set_local_position(Vector3 const& newPos);

		[[nodiscard]] LEOPPHAPI Quaternion const& get_rotation() const;
		LEOPPHAPI void set_rotation(Quaternion const& newRot);

		[[nodiscard]] LEOPPHAPI Quaternion const& get_local_rotation() const;
		LEOPPHAPI void set_local_rotation(Quaternion const& newRot);

		[[nodiscard]] LEOPPHAPI Vector3 const& get_scale() const;
		LEOPPHAPI void set_scale(Vector3 const& newScale);

		[[nodiscard]] LEOPPHAPI Vector3 const& get_local_scale() const;
		LEOPPHAPI void set_local_scale(Vector3 const& newScale);

		LEOPPHAPI void translate(Vector3 const& vector, Space base = Space::World);
		LEOPPHAPI void translate(f32 x, f32 y, f32 z, Space base = Space::World);

		LEOPPHAPI void rotate(Quaternion const& rotation, Space base = Space::World);
		LEOPPHAPI void rotate(Vector3 const& axis, f32 angleDegrees, Space base = Space::World);

		LEOPPHAPI void rescale(Vector3 const& scaling, Space base = Space::World);
		LEOPPHAPI void rescale(f32 x, f32 y, f32 z, Space base = Space::World);

		[[nodiscard]] LEOPPHAPI Vector3 const& get_forward_axis() const;
		[[nodiscard]] LEOPPHAPI Vector3 const& get_right_axis() const;
		[[nodiscard]] LEOPPHAPI Vector3 const& get_up_axis() const;

		[[nodiscard]] LEOPPHAPI Node* get_parent() const;
		void LEOPPHAPI set_parent(Node* parent);
		void LEOPPHAPI unparent();

		[[nodiscard]] LEOPPHAPI std::span<Node* const> get_children() const;

		LEOPPHAPI Matrix4 get_model_matrix() const;
		LEOPPHAPI Matrix3 get_normal_matrix() const;

	private:
		void calculate_world_position_and_update_children();
		void calculate_world_rotation_and_update_children();
		void calculate_world_scale_and_update_children();
		void calculate_matrices();
		void init();
		void deinit();
		void take_children_from(Node const& node);
	};

	LEOPPHAPI extern std::unordered_map<u64, std::unique_ptr<Node>> nodes;
}