#include "Transform.hpp"

#include "Entity.hpp"


namespace leopph
{
	Vector3 const& Transform::get_position() const
	{
		return mWorldPosition;
	}



	void Transform::set_position(Vector3 const& newPos)
	{
		if (mParent != nullptr)
		{
			mLocalPosition = mParent->mWorldRotation.Conjugate().Rotate(newPos) - mParent->mWorldPosition;
		}
		else
		{
			mLocalPosition = newPos;
		}
		calculate_world_position_and_update_children();
	}



	Vector3 const& Transform::get_local_position() const
	{
		return mLocalPosition;
	}



	void Transform::set_local_position(Vector3 const& newPos)
	{
		mLocalPosition = newPos;
		calculate_world_position_and_update_children();
	}



	Quaternion const& Transform::get_rotation() const
	{
		return mWorldRotation;
	}



	void Transform::set_rotation(Quaternion const& newRot)
	{
		if (mParent != nullptr)
		{
			mLocalRotation = mParent->mWorldRotation.Conjugate() * newRot;
		}
		else
		{
			mLocalRotation = newRot;
		}
		calculate_world_rotation_and_update_children();
	}



	Quaternion const& Transform::get_local_rotation() const
	{
		return mLocalRotation;
	}



	void Transform::set_local_rotation(Quaternion const& newRot)
	{
		mLocalRotation = newRot;
		calculate_world_rotation_and_update_children();
	}



	Vector3 const& Transform::get_scale() const
	{
		return mWorldScale;
	}



	void Transform::set_scale(Vector3 const& newScale)
	{
		if (mParent != nullptr)
		{
			mLocalScale = newScale / mParent->mWorldScale;
		}
		else
		{
			mLocalScale = newScale;
		}
		calculate_world_scale_and_update_children();
	}



	Vector3 const& Transform::get_local_scale() const
	{
		return mLocalScale;
	}



	void Transform::set_local_scale(Vector3 const& newScale)
	{
		mLocalScale = newScale;
		calculate_world_scale_and_update_children();
	}



	void Transform::translate(Vector3 const& vector, Space const base)
	{
		if (base == Space::World)
		{
			set_position(mWorldPosition + vector);
		}
		else if (base == Space::Local)
		{
			set_local_position(mLocalPosition + (vector * static_cast<Matrix3>(static_cast<Matrix4>(mLocalRotation))));
		}
	}



	void Transform::translate(f32 const x, f32 const y, f32 const z, Space const base)
	{
		translate(Vector3{x, y, z}, base);
	}



	void Transform::rotate(Quaternion const& rotation, Space const base)
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



	void Transform::rotate(Vector3 const& axis, f32 const amountDegrees, Space const base)
	{
		rotate(Quaternion{axis, amountDegrees}, base);
	}



	void Transform::rescale(Vector3 const& scaling, Space const base)
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



	void Transform::rescale(f32 const x, f32 const y, f32 const z, Space const base)
	{
		rescale(Vector3{x, y, z}, base);
	}



	Vector3 const& Transform::get_forward_axis() const
	{
		return mForward;
	}



	Vector3 const& Transform::get_right_axis() const
	{
		return mRight;
	}



	Vector3 const& Transform::get_up_axis() const
	{
		return mUp;
	}



	Transform* Transform::get_parent() const
	{
		return mParent;
	}



	void Transform::set_parent(Entity* const parent)
	{
		if (parent)
		{
			set_parent(parent->get_transform());
		}
		else
		{
			unparent();
		}
	}



	void Transform::set_parent(Transform* parent)
	{
		if (parent)
		{
			set_parent(*parent);
		}
		else
		{
			unparent();
		}
	}



	void Transform::set_parent(Transform& parent)
	{
		unparent();

		mParent = &parent;
		mParent->mChildren.push_back(this);

		calculate_world_position_and_update_children();
		calculate_world_rotation_and_update_children();
		calculate_world_scale_and_update_children();
	}



	void Transform::unparent()
	{
		if (mParent != nullptr)
		{
			std::erase(mParent->mChildren, this);
			mParent = nullptr;
		}
	}



	std::span<Transform* const> Transform::get_children() const
	{
		return mChildren;
	}



	Matrix4 Transform::get_model_matrix() const
	{
		return mModelMat;
	}



	Matrix4 Transform::get_normal_matrix() const
	{
		return mNormalMat;
	}



	Entity* Transform::get_entity() const
	{
		return mEntity;
	}



	void Transform::calculate_world_position_and_update_children()
	{
		mWorldPosition = mParent != nullptr ? mParent->mWorldRotation.Rotate(mParent->mWorldPosition + mLocalPosition) : mLocalPosition;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_position_and_update_children();
		}
	}



	void Transform::calculate_world_rotation_and_update_children()
	{
		mWorldRotation = mParent != nullptr ? mParent->mWorldRotation * mLocalRotation : mLocalRotation;

		mForward = mWorldRotation.Rotate(Vector3::Forward());
		mRight = mWorldRotation.Rotate(Vector3::Right());
		mUp = mWorldRotation.Rotate(Vector3::Up());

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_rotation_and_update_children();
		}
	}



	void Transform::calculate_world_scale_and_update_children()
	{
		mWorldScale = mParent != nullptr ? mParent->mWorldScale * mLocalScale : mLocalScale;

		calculate_matrices();

		for (auto* const child : mChildren)
		{
			child->calculate_world_scale_and_update_children();
		}
	}



	void Transform::calculate_matrices()
	{
		mModelMat[0] = Vector4{mRight * mWorldScale[0], 0};
		mModelMat[1] = Vector4{mUp * mWorldScale[1], 0};
		mModelMat[2] = Vector4{mForward * mWorldScale[2], 0};
		mModelMat[3] = Vector4{mWorldPosition, 1};

		auto const worldScaleRecip{1.f / mWorldScale};

		mNormalMat[0] = Vector4{mRight * worldScaleRecip[0], -mWorldPosition[0]};
		mNormalMat[1] = Vector4{mUp * worldScaleRecip[1], -mWorldPosition[1]};
		mNormalMat[2] = Vector4{mForward * worldScaleRecip[2], -mWorldPosition[2]};
		mNormalMat[3] = Vector4{0, 0, 0, 1};
	}



	Transform::Transform(Entity* entity) :
		mEntity{entity}
	{ }



	Transform::~Transform()
	{
		unparent();

		for (auto* const child : mChildren)
		{
			// Can't call parent setter, because that would remove child from this's mChildren, and the loop would crash.
			child->mParent = nullptr;
			child->calculate_world_position_and_update_children();
			child->calculate_world_rotation_and_update_children();
			child->calculate_world_scale_and_update_children();
		}
	}
}
