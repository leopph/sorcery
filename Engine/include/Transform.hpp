#pragma once

#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Types.hpp"
#include "Vector.hpp"

#include <span>
#include <vector>


namespace leopph
{
	class Entity;


	enum class Space
	{
		World,
		Local
	};


	class Transform final
	{
		public:
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
			LEOPPHAPI void rotate(Vector3 const& axis, f32 amountDegrees, Space base = Space::World);


			LEOPPHAPI void rescale(Vector3 const& scaling, Space base = Space::World);
			LEOPPHAPI void rescale(f32 x, f32 y, f32 z, Space base = Space::World);


			[[nodiscard]] LEOPPHAPI Vector3 const& get_forward_axis() const;
			[[nodiscard]] LEOPPHAPI Vector3 const& get_right_axis() const;
			[[nodiscard]] LEOPPHAPI Vector3 const& get_up_axis() const;


			[[nodiscard]] LEOPPHAPI Transform* get_parent() const;
			void LEOPPHAPI set_parent(Entity* parent);
			void LEOPPHAPI set_parent(Transform* parent);
			void LEOPPHAPI set_parent(Transform& parent);
			void LEOPPHAPI unparent();


			[[nodiscard]] LEOPPHAPI std::span<Transform* const> get_children() const;


			LEOPPHAPI Matrix4 get_model_matrix() const;
			LEOPPHAPI Matrix4 get_normal_matrix() const;


			[[nodiscard]] LEOPPHAPI Entity* get_entity() const;


		private:
			void calculate_world_position_and_update_children();
			void calculate_world_rotation_and_update_children();
			void calculate_world_scale_and_update_children();
			void calculate_matrices();


		public:
			LEOPPHAPI explicit Transform(Entity* entity);

			Transform(Transform const& other) = delete;
			void operator=(Transform const& other) = delete;

			Transform(Transform&&) = delete;
			void operator=(Transform&&) = delete;

			LEOPPHAPI ~Transform();


		private:
			Vector3 mWorldPosition;
			Quaternion mWorldRotation;
			Vector3 mWorldScale{1};

			Vector3 mLocalPosition;
			Quaternion mLocalRotation;
			Vector3 mLocalScale{1};

			Vector3 mForward{Vector3::Forward()};
			Vector3 mRight{Vector3::Right()};
			Vector3 mUp{Vector3::Up()};

			Transform* mParent{nullptr};
			std::vector<Transform*> mChildren;

			Entity* mEntity;

			Matrix4 mModelMat;
			Matrix4 mNormalMat;
	};
}
