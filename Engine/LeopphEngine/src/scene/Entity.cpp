#include "Entity.hpp"

#include "Context.hpp"
#include "SceneManager.hpp"
#include "Types.hpp"

#include <utility>


namespace leopph
{
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
		else if (base == Space::Local)
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
		else if (base == Space::Local)
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
		else if (base == Space::Local)
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



	void Entity::remove_component(Component& component)
	{
		std::erase_if(mComponents, [&component](auto const& elem)
		{
			return elem.get() == &component;
		});
	}



	Entity::Entity() :
		mScene{&get_scene_manager().get_active_scene()}
	{
		mScene->register_entity(this);
	}



	Entity::~Entity()
	{
		unparent();

		// Unparent removes the child from mChildren so we cannot iterate over it
		while (!mChildren.empty())
		{
			mChildren.back()->unparent();
		}

		mScene->unregister_entity(this);
	}
}
