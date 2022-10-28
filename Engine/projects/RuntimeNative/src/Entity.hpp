#pragma once

#include "Math.hpp"
#include "ManagedAccessObject.hpp"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>
#include <span>
#include <vector>

using MonoObject = struct _MonoObject;


namespace leopph
{
	class Component;

	enum class Space : u8
	{
		World = 0,
		Object = 1
	};


	class Entity : public ManagedAccessObject
	{
	private:
		//Scene* mScene;
		std::string mName{"Entity"};

		Vector3 mLocalPosition{0};
		Quaternion mLocalRotation{1, 0, 0, 0};
		Vector3 mLocalScale{1};

		Vector3 mWorldPosition{mLocalPosition};
		Quaternion mWorldRotation{mLocalRotation};
		Vector3 mWorldScale{mLocalScale};

		Vector3 mForward{Vector3::forward()};
		Vector3 mRight{Vector3::right()};
		Vector3 mUp{Vector3::up()};

		Entity* mParent{nullptr};
		std::vector<Entity*> mChildren;

		Matrix4 mModelMat{Matrix4::identity()};
		Matrix3 mNormalMat{Matrix4::identity()};

		std::vector<Component*> mComponents;

	public:
		LEOPPHAPI Entity();
		LEOPPHAPI Entity(MonoObject* managedObject);
		Entity(Entity const& other) = delete;
		Entity(Entity&& other) = delete;

		Entity& operator=(Entity const& other) = delete;
		Entity& operator=(Entity&& other) = delete;

		LEOPPHAPI ~Entity();

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

		[[nodiscard]] LEOPPHAPI Entity* get_parent() const;
		void LEOPPHAPI set_parent(Entity* parent);
		void LEOPPHAPI unparent();

		[[nodiscard]] LEOPPHAPI std::span<Entity* const> get_children() const;

		LEOPPHAPI Matrix4 get_model_matrix() const;
		LEOPPHAPI Matrix3 get_normal_matrix() const;

		[[nodiscard]] std::span<Component* const> get_components() const;
		void add_component(Component* component);
		void remove_component(Component* component);

	private:
		void calculate_world_position_and_update_children();
		void calculate_world_rotation_and_update_children();
		void calculate_world_scale_and_update_children();
		void calculate_matrices();
		void init();
		void take_children_from(Entity const& node);
	};


	LEOPPHAPI extern std::vector<Entity*> gEntities;


	namespace managedbindings
	{
		void entity_new(MonoObject* managedEntity);

		Vector3 const* entity_get_world_position(Entity* const entity);
		void set_entity_world_position(Entity* entity, Vector3 const* position);
		Vector3 const* get_entity_local_position(Entity* entity);
		void set_entity_local_position(Entity* entity, Vector3 const* position);

		Quaternion const* get_entity_world_rotation(Entity* entity);
		void set_entity_world_rotation(Entity* entity, Quaternion const* rotation);
		Quaternion const* get_entity_local_rotation(Entity* entity);
		void set_entity_local_rotation(Entity* entity, Quaternion const* rotation);

		Vector3 const* get_entity_world_scale(Entity* entity);
		void set_entity_world_scale(Entity* entity, Vector3 const* scale);
		Vector3 const* get_entity_local_scale(Entity* entity);
		void set_entity_local_scale(Entity* entity, Vector3 const* scale);

		void translate_entity_from_vector(Entity* entity, Vector3 const* translation, Space space);
		void translate_entity(Entity* entity, f32 x, f32 y, f32 z, Space space);

		void rotate_entity(Entity* entity, Quaternion const* rotation, Space space);
		void rotate_entity_angle_axis(Entity* entity, Vector3 const* axis, f32 angleDegrees, Space space);

		void rescale_entity_from_vector(Entity* entity, Vector3 const* scaling, Space space);
		void rescale_entity(Entity* entity, f32 x, f32 y, f32 z, Space space);

		Vector3 const* get_entity_right_axis(Entity* entity);
		Vector3 const* get_entity_up_axis(Entity* entity);
		Vector3 const* get_entity_forward_axis(Entity* entity);

		u64 get_entity_parent_handle(Entity* entity);
		void set_entity_parent(Entity* targetEntity, Entity* parentEntity);

		u64 get_entity_child_count(Entity* entity);
		u64 get_entity_child_handle(Entity* entity, u64 childIndex);
	}
}