#include "Entity.hpp"

namespace leopph
{
	u64 Entity::sNextId{1};


	Entity::Entity() //:
		//mScene{&get_scene_manager().get_active_scene()}
	{
		//mScene->add(this);
	}


	Entity::Entity(Entity const& other) :
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


	Entity::Entity(Entity&& other) noexcept :
		Entity{other}
	{
		take_children_from(other);
	}


	Entity& Entity::operator=(Entity const& other)
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


	Entity& Entity::operator=(Entity&& other) noexcept
	{
		if (this == &other)
		{
			return *this;
		}

		*this = other;
		take_children_from(other);

		return *this;
	}


	Entity::~Entity()
	{
		deinit();
	}


	u64 Entity::get_id() const
	{
		return mId;
	}


	std::string_view Entity::get_name() const
	{
		return mName;
	}


	void Entity::set_name(std::string name)
	{
		mName = std::move(name);
	}


	Vector3 const& Entity::get_position() const
	{
		return mWorldPosition;
	}


	void Entity::set_position(Vector3 const& newPos)
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


	Vector3 const& Entity::get_local_position() const
	{
		return mLocalPosition;
	}


	void Entity::set_local_position(Vector3 const& newPos)
	{
		mLocalPosition = newPos;
		calculate_world_position_and_update_children();
	}


	Quaternion const& Entity::get_rotation() const
	{
		return mWorldRotation;
	}


	void Entity::set_rotation(Quaternion const& newRot)
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


	Quaternion const& Entity::get_local_rotation() const
	{
		return mLocalRotation;
	}


	void Entity::set_local_rotation(Quaternion const& newRot)
	{
		mLocalRotation = newRot;
		calculate_world_rotation_and_update_children();
	}


	Vector3 const& Entity::get_scale() const
	{
		return mWorldScale;
	}


	void Entity::set_scale(Vector3 const& newScale)
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


	Vector3 const& Entity::get_local_scale() const
	{
		return mLocalScale;
	}


	void Entity::set_local_scale(Vector3 const& newScale)
	{
		mLocalScale = newScale;
		calculate_world_scale_and_update_children();
	}


	void Entity::translate(Vector3 const& vector, Space const base)
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


	void Entity::translate(f32 const x, f32 const y, f32 const z, Space const base)
	{
		translate(Vector3{x, y, z}, base);
	}


	void Entity::rotate(Quaternion const& rotation, Space const base)
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


	void Entity::rotate(Vector3 const& axis, f32 const amountDegrees, Space const base)
	{
		rotate(Quaternion{axis, amountDegrees}, base);
	}


	void Entity::rescale(Vector3 const& scaling, Space const base)
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


	void Entity::rescale(f32 const x, f32 const y, f32 const z, Space const base)
	{
		rescale(Vector3{x, y, z}, base);
	}


	Vector3 const& Entity::get_forward_axis() const
	{
		return mForward;
	}


	Vector3 const& Entity::get_right_axis() const
	{
		return mRight;
	}


	Vector3 const& Entity::get_up_axis() const
	{
		return mUp;
	}


	Entity* Entity::get_parent() const
	{
		return mParent;
	}


	void Entity::set_parent(Entity* const parent)
	{
		unparent();

		mParent = parent;
		mParent->mChildren.push_back(this);

		calculate_world_position_and_update_children();
		calculate_world_rotation_and_update_children();
		calculate_world_scale_and_update_children();
	}


	void Entity::unparent()
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


	std::span<Entity* const> Entity::get_children() const
	{
		return mChildren;
	}


	Matrix4 Entity::get_model_matrix() const
	{
		return mModelMat;
	}


	Matrix3 Entity::get_normal_matrix() const
	{
		return mNormalMat;
	}


	void Entity::calculate_world_position_and_update_children()
	{
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_position_and_update_children();
		}
	}


	void Entity::calculate_world_rotation_and_update_children()
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


	void Entity::calculate_world_scale_and_update_children()
	{
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_scale_and_update_children();
		}
	}


	void Entity::calculate_matrices()
	{
		mModelMat[0] = Vector4{mRight * mWorldScale, 0};
		mModelMat[1] = Vector4{mUp * mWorldScale, 0};
		mModelMat[2] = Vector4{mForward * mWorldScale, 0};
		mModelMat[3] = Vector4{mWorldPosition, 1};

		mNormalMat[0] = mRight / mWorldScale;
		mNormalMat[1] = mUp / mWorldScale;
		mNormalMat[2] = mForward / mWorldScale;
	}


	void Entity::init()
	{
		//mScene->add(this);

		if (mParent)
		{
			mParent->mChildren.emplace_back(this);
		}
	}


	void Entity::deinit()
	{
		unparent();

		// Unparent removes the child from mChildren so we cannot iterate over it
		while (!mChildren.empty())
		{
			mChildren.back()->unparent();
		}

		//mScene->remove(this);
	}


	void Entity::take_children_from(Entity const& entity)
	{
		while (!entity.mChildren.empty())
		{
			auto* child = entity.mChildren.back();
			child->unparent();
			child->set_parent(this);
		}
	}


	std::unordered_map<u64, std::unique_ptr<Entity>> entities;


	namespace detail
	{
		u64 new_entity()
		{
			static u64 nextId{1};
			u64 const id{nextId++};
			entities[id] = std::make_unique<Entity>();
			return id;
		}


		i32 is_entity_alive(u64 const id)
		{
			return entities[id] != nullptr;
		}


		void delete_entity(u64 const id)
		{
			entities[id].reset();
		}


		Vector3 const* get_entity_world_position(u64 const id)
		{
			return &entities[id]->get_position();
		}


		void set_entity_world_position(u64 const id, Vector3 const* position)
		{
			entities[id]->set_position(*position);
		}


		Vector3 const* get_entity_local_position(u64 const id)
		{
			return &entities[id]->get_local_position();
		}


		void set_entity_local_position(u64 const id, Vector3 const* position)
		{
			entities[id]->set_local_position(*position);
		}


		Quaternion const* get_entity_world_rotation(u64 const id)
		{
			return &entities[id]->get_rotation();
		}


		void set_entity_world_rotation(u64 const id, Quaternion const* rotation)
		{
			entities[id]->set_rotation(*rotation);
		}


		Quaternion const* get_entity_local_rotation(u64 const id)
		{
			return &entities[id]->get_local_rotation();
		}


		void set_entity_local_rotation(u64 const id, Quaternion const* rotation)
		{
			entities[id]->set_local_rotation(*rotation);
		}


		Vector3 const* get_entity_world_scale(u64 const id)
		{
			return &entities[id]->get_scale();
		}


		void set_entity_world_scale(u64 const id, Vector3 const* scale)
		{
			entities[id]->set_scale(*scale);
		}


		Vector3 const* get_entity_local_scale(u64 const id)
		{
			return &entities[id]->get_local_scale();
		}


		void set_entity_local_scale(u64 const id, Vector3 const* scale)
		{
			entities[id]->set_local_scale(*scale);
		}


		void translate_entity_from_vector(u64 const id, Vector3 const* translation, Space const space)
		{
			entities[id]->translate(*translation, space);
		}


		void translate_entity(u64 const id, f32 const x, f32 const y, f32 const z, Space const space)
		{
			entities[id]->translate(x, y, z, space);
		}


		void rotate_entity(u64 const id, Quaternion const* rotation, Space const space)
		{
			entities[id]->rotate(*rotation, space);
		}


		void rotate_entity_angle_axis(u64 const id, Vector3 const* axis, f32 const angleDegrees, Space const space)
		{
			entities[id]->rotate(*axis, angleDegrees, space);
		}


		void rescale_entity_from_vector(u64 const id, Vector3 const* scaling, Space const space)
		{
			entities[id]->rescale(*scaling, space);
		}


		void rescale_entity(u64 const id, f32 const x, f32 const y, f32 const z, Space const space)
		{
			entities[id]->rescale(x, y, z, space);
		}


		Vector3 const* get_entity_right_axis(u64 const id)
		{
			return &entities[id]->get_right_axis();
		}


		Vector3 const* get_entity_up_axis(u64 const id)
		{
			return &entities[id]->get_up_axis();
		}


		Vector3 const* get_entity_forward_axis(u64 const id)
		{
			return &entities[id]->get_forward_axis();
		}


		u64 get_entity_parent_id(u64 const id)
		{
			Entity* parent = entities[id]->get_parent();
			return parent ? parent->get_id() : 0;
		}


		void set_entity_parent(u64 const targetEntityId, u64 const parentEntityId)
		{
			entities[targetEntityId]->set_parent(parentEntityId == 0 ? nullptr : entities[parentEntityId].get());
		}


		u64 get_entity_child_count(u64 const id)
		{
			return entities[id]->get_children().size();
		}


		u64 get_entity_child_id(u64 const parentId, u64 const childIndex)
		{
			return entities[parentId]->get_children()[childIndex]->get_id();
		}
	}
}