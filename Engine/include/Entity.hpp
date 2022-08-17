#pragma once

#include "Component.hpp"
#include "LeopphApi.hpp"
#include "Matrix.hpp"
#include "Quaternion.hpp"
#include "Vector.hpp"

#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>


namespace leopph
{
	class Scene;


	enum class Space
	{
		World,
		Local
	};


	class Entity
	{
		public:
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
			LEOPPHAPI void rotate(Vector3 const& axis, f32 amountDegrees, Space base = Space::World);


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


		private:
			void calculate_world_position_and_update_children();
			void calculate_world_rotation_and_update_children();
			void calculate_world_scale_and_update_children();
			void calculate_matrices();


		public:
			template<std::derived_from<Component> T>
			[[nodiscard]] T* get_component() const;

			template<std::derived_from<Component> T, class... Args>
			T& attach_component(Args&&... args);

			void remove_component(Component& component);


			LEOPPHAPI Entity();

			Entity(Entity const& other) = delete;
			Entity& operator=(Entity const& other) = delete;

			Entity(Entity&& other) = delete;
			void operator=(Entity&& other) = delete;

			LEOPPHAPI ~Entity();


		private:
			Scene* mScene;
			std::string mName{"Entity"};
			std::vector<std::unique_ptr<Component>> mComponents;

			Vector3 mWorldPosition;
			Quaternion mWorldRotation;
			Vector3 mWorldScale{1};

			Vector3 mLocalPosition;
			Quaternion mLocalRotation;
			Vector3 mLocalScale{1};

			Vector3 mForward{Vector3::forward()};
			Vector3 mRight{Vector3::right()};
			Vector3 mUp{Vector3::up()};

			Entity* mParent{nullptr};
			std::vector<Entity*> mChildren;

			Matrix4 mModelMat;
			Matrix3 mNormalMat;
	};



	template<std::derived_from<Component> T, class... Args>
	T& Entity::attach_component(Args&&... args)
	{
		auto comp = std::make_unique<T>(std::forward<Args>(args)...);
		comp->mOwner = this;
		auto& ret = *comp;
		mComponents.emplace_back(std::move(comp));
		return ret;
	}



	template<std::derived_from<Component> T>
	T* Entity::get_component() const
	{
		for (auto const& component : mComponents)
		{
			if (auto ret = std::dynamic_pointer_cast<T>(component))
			{
				return ret;
			}
		}

		return nullptr;
	}
}
