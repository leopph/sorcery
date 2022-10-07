#include "Node.hpp"

namespace leopph
{
	u64 Node::sNextId{1};


	Node::Node() //:
		//mScene{&get_scene_manager().get_active_scene()}
	{
		//mScene->add(this);
	}


	Node::Node(Node const& other) :
		//mScene{other.mScene},
		mName{other.mName},
		mLocalPosition{other.mLocalPosition},
		mLocalScale{other.mLocalScale},
		mWorldPosition{other.mWorldPosition},
		mWorldRotation{other.mWorldRotation},
		mWorldScale{other.mWorldScale},
		mForward{other.mForward},
		mRight{other.mRight},
		mUp{other.mUp},
		mParent{other.mParent},
		mModelMat{other.mModelMat},
		mNormalMat{other.mNormalMat}
	{
		init();
	}


	Node::Node(Node&& other) noexcept :
		Node{other}
	{
		take_children_from(other);
	}


	Node& Node::operator=(Node const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		deinit();

		//mScene = other.mScene;
		mName = other.mName;
		mLocalPosition = other.mLocalPosition;
		mLocalRotation = other.mLocalRotation;
		mLocalScale = other.mLocalScale;
		mWorldPosition = other.mWorldPosition;
		mWorldRotation = other.mWorldRotation;
		mWorldScale = other.mWorldScale;
		mForward = other.mForward;
		mRight = other.mRight;
		mUp = other.mUp;
		mParent = other.mParent;
		mModelMat = other.mModelMat;
		mNormalMat = other.mNormalMat;

		init();

		return *this;
	}


	Node& Node::operator=(Node&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		*this = other;
		take_children_from(other);

		return *this;
	}


	Node::~Node()
	{
		deinit();
	}


	Vector3 const& Node::get_position() const
	{
		return mWorldPosition;
	}


	void Node::set_position(Vector3 const& newPos)
	{
		if (mParent != nullptr)
		{
			set_local_position(mParent->mWorldRotation.conjugate().rotate(newPos) - mParent->mWorldPosition);
		}
		else
		{
			set_local_position(newPos);
		}
	}


	Vector3 const& Node::get_local_position() const
	{
		return mLocalPosition;
	}


	void Node::set_local_position(Vector3 const& newPos)
	{
		mLocalPosition = newPos;
		calculate_world_position_and_update_children();
	}


	Quaternion const& Node::get_rotation() const
	{
		return mWorldRotation;
	}


	void Node::set_rotation(Quaternion const& newRot)
	{
		if (mParent != nullptr)
		{
			set_local_rotation(mParent->mWorldRotation.conjugate() * newRot);
		}
		else
		{
			set_local_rotation(newRot);
		}
	}


	Quaternion const& Node::get_local_rotation() const
	{
		return mLocalRotation;
	}


	void Node::set_local_rotation(Quaternion const& newRot)
	{
		mLocalRotation = newRot;
		calculate_world_rotation_and_update_children();
	}


	Vector3 const& Node::get_scale() const
	{
		return mWorldScale;
	}


	void Node::set_scale(Vector3 const& newScale)
	{
		if (mParent != nullptr)
		{
			set_local_scale(newScale / mParent->mWorldScale);
		}
		else
		{
			set_local_scale(newScale);
		}
	}


	Vector3 const& Node::get_local_scale() const
	{
		return mLocalScale;
	}


	void Node::set_local_scale(Vector3 const& newScale)
	{
		mLocalScale = newScale;
		calculate_world_scale_and_update_children();
	}


	void Node::translate(Vector3 const& vector, Space const base)
	{
		if (base == Space::World)
		{
			set_position(mWorldPosition + vector);
		}
		else if (base == Space::Object)
		{
			set_local_position(mLocalPosition + mLocalRotation.rotate(vector));
		}
	}


	void Node::translate(f32 const x, f32 const y, f32 const z, Space const base)
	{
		translate(Vector3{x, y, z}, base);
	}


	void Node::rotate(Quaternion const& rotation, Space const base)
	{
		if (base == Space::World)
		{
			set_local_rotation(rotation * mLocalRotation);
		}
		else if (base == Space::Object)
		{
			set_local_rotation(mLocalRotation * rotation);
		}
	}


	void Node::rotate(Vector3 const& axis, f32 const amountDegrees, Space const base)
	{
		rotate(Quaternion{axis, amountDegrees}, base);
	}


	void Node::rescale(Vector3 const& scaling, Space const base)
	{
		if (base == Space::World)
		{
			set_scale(mWorldScale * scaling);
		}
		else if (base == Space::Object)
		{
			set_local_scale(mLocalScale * scaling);
		}
	}


	void Node::rescale(f32 const x, f32 const y, f32 const z, Space const base)
	{
		rescale(Vector3{x, y, z}, base);
	}


	Vector3 const& Node::get_forward_axis() const
	{
		return mForward;
	}


	Vector3 const& Node::get_right_axis() const
	{
		return mRight;
	}


	Vector3 const& Node::get_up_axis() const
	{
		return mUp;
	}


	Node* Node::get_parent() const
	{
		return mParent;
	}


	void Node::set_parent(Node* const parent)
	{
		unparent();

		mParent = parent;
		mParent->mChildren.push_back(this);

		calculate_world_position_and_update_children();
		calculate_world_rotation_and_update_children();
		calculate_world_scale_and_update_children();
	}


	void Node::unparent()
	{
		if (mParent != nullptr)
		{
			std::erase(mParent->mChildren, this);
			mParent = nullptr;

			calculate_world_position_and_update_children();
			calculate_world_rotation_and_update_children();
			calculate_world_scale_and_update_children();
		}
	}


	std::span<Node* const> Node::get_children() const
	{
		return mChildren;
	}


	Matrix4 Node::get_model_matrix() const
	{
		return mModelMat;
	}


	Matrix3 Node::get_normal_matrix() const
	{
		return mNormalMat;
	}


	void Node::calculate_world_position_and_update_children()
	{
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_position_and_update_children();
		}
	}


	void Node::calculate_world_rotation_and_update_children()
	{
		mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;

		mForward = mWorldRotation.rotate(Vector3::forward());
		mRight = mWorldRotation.rotate(Vector3::right());
		mUp = mWorldRotation.rotate(Vector3::up());

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_rotation_and_update_children();
		}
	}


	void Node::calculate_world_scale_and_update_children()
	{
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_scale_and_update_children();
		}
	}


	void Node::calculate_matrices()
	{
		mModelMat[0] = Vector4{mRight * mWorldScale, 0};
		mModelMat[1] = Vector4{mUp * mWorldScale, 0};
		mModelMat[2] = Vector4{mForward * mWorldScale, 0};
		mModelMat[3] = Vector4{mWorldPosition, 1};

		mNormalMat[0] = mRight / mWorldScale;
		mNormalMat[1] = mUp / mWorldScale;
		mNormalMat[2] = mForward / mWorldScale;
	}


	void Node::init()
	{
		//mScene->add(this);

		if (mParent)
		{
			mParent->mChildren.emplace_back(this);
		}
	}


	void Node::deinit()
	{
		unparent();

		// Unparent removes the child from mChildren so we cannot iterate over it
		while (!mChildren.empty())
		{
			mChildren.back()->unparent();
		}

		//mScene->remove(this);
	}


	void Node::take_children_from(Node const& node)
	{
		while (!node.mChildren.empty())
		{
			auto* child = node.mChildren.back();
			child->unparent();
			child->set_parent(this);
		}
	}


	std::unordered_map<u64, std::unique_ptr<Node>> nodes;


	extern "C"
	{
		__declspec(dllexport) u64 new_node()
		{
			static u64 nextNodeId{1};
			u64 const id{nextNodeId++};
			nodes[id] = std::make_unique<Node>();
			return id;
		}


		__declspec (dllexport) i32 is_node_alive(u64 const id)
		{
			return nodes[id] != nullptr;
		}


		__declspec(dllexport) void delete_node(u64 const id)
		{
			nodes[id].reset();
		}


		__declspec(dllexport) Vector3 const* get_node_world_position(u64 const id)
		{
			return &nodes[id]->get_position();
		}


		__declspec(dllexport) void set_node_world_position(u64 const id, Vector3 const* position)
		{
			nodes[id]->set_position(*position);
		}


		__declspec(dllexport) Vector3 const* get_node_local_position(u64 const id)
		{
			return &nodes[id]->get_local_position();
		}


		__declspec(dllexport) void set_node_local_position(u64 const id, Vector3 const* position)
		{
			nodes[id]->set_local_position(*position);
		}


		__declspec(dllexport) Quaternion const* get_node_world_rotation(u64 const id)
		{
			return &nodes[id]->get_rotation();
		}


		__declspec(dllexport) void set_node_world_rotation(u64 const id, Quaternion const* rotation)
		{
			nodes[id]->set_rotation(*rotation);
		}


		__declspec(dllexport) Quaternion const* get_node_local_rotation(u64 const id)
		{
			return &nodes[id]->get_local_rotation();
		}


		__declspec(dllexport) void set_node_local_rotation(u64 const id, Quaternion const* rotation)
		{
			nodes[id]->set_local_rotation(*rotation);
		}


		__declspec(dllexport) Vector3 const* get_node_world_scale(u64 const id)
		{
			return &nodes[id]->get_scale();
		}


		__declspec(dllexport) void set_node_world_scale(u64 const id, Vector3 const* scale)
		{
			nodes[id]->set_scale(*scale);
		}


		__declspec(dllexport) Vector3 const* get_node_local_scale(u64 const id)
		{
			return &nodes[id]->get_local_scale();
		}


		__declspec(dllexport) void set_node_local_scale(u64 const id, Vector3 const* scale)
		{
			nodes[id]->set_local_scale(*scale);
		}


		__declspec(dllexport) void translate_node_from_vector(u64 const id, Vector3 const* translation, Space const space)
		{
			nodes[id]->translate(*translation, space);
		}


		__declspec(dllexport) void translate_node(u64 const id, f32 const x, f32 const y, f32 const z, Space const space)
		{
			nodes[id]->translate(x, y, z, space);
		}


		__declspec(dllexport) void rotate_node(u64 const id, Quaternion const* rotation, Space const space)
		{
			nodes[id]->rotate(*rotation, space);
		}


		__declspec(dllexport) void rotate_node_angle_axis(u64 const id, Vector3 const* axis, f32 const angleDegrees, Space const space)
		{
			nodes[id]->rotate(*axis, angleDegrees, space);
		}


		__declspec(dllexport) void rescale_node_from_vector(u64 const id, Vector3 const* scaling, Space const space)
		{
			nodes[id]->rescale(*scaling, space);
		}


		__declspec(dllexport) void rescale_node(u64 const id, f32 const x, f32 const y, f32 const z, Space const space)
		{
			nodes[id]->rescale(x, y, z, space);
		}


		__declspec(dllexport) Vector3 const* get_node_right_axis(u64 const id)
		{
			return &nodes[id]->get_right_axis();
		}


		__declspec(dllexport) Vector3 const* get_node_up_axis(u64 const id)
		{
			return &nodes[id]->get_up_axis();
		}


		__declspec(dllexport) Vector3 const* get_node_forward_axis(u64 const id)
		{
			return &nodes[id]->get_forward_axis();
		}
	}
}