#pragma once

#include "Component.hpp"
#include "Math.hpp"
#include "Types.hpp"


namespace leopph
{
	class Light : public Component
	{
		public:
			[[nodiscard]] LEOPPHAPI Vector3 const& get_color() const;
			LEOPPHAPI void set_color(Vector3 const& newColor);

			[[nodiscard]] LEOPPHAPI f32 get_intensity() const;
			LEOPPHAPI void set_intensity(f32 newIntensity);

			[[nodiscard]] LEOPPHAPI bool is_casting_shadow() const;
			LEOPPHAPI void set_casting_shadow(bool newValue);

		protected:
			Light() = default;
		public:
			Light(Light const& other) = delete;
			Light(Light&& other) = delete;

			~Light() override = default;

			Light& operator=(Light const& other) = delete;
			Light& operator=(Light&& other) = delete;

		private:
			bool mCastsShadow{false};
			Vector3 mColor{1.f};
			f32 mIntensity{1.f};
	};


	class AttenuatedLight : public Light
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_range() const;
			LEOPPHAPI void set_range(f32 value);


		protected:
			AttenuatedLight() = default;
		public:
			AttenuatedLight(AttenuatedLight const& other) = delete;
			AttenuatedLight(AttenuatedLight&& other) = delete;

			~AttenuatedLight() override = default;

			AttenuatedLight& operator=(AttenuatedLight const& other) = delete;
			AttenuatedLight& operator=(AttenuatedLight&& other) = delete;

		private:
			f32 mRange{10.f};
	};


	class AmbientLight final
	{
		public:
			LEOPPHAPI static AmbientLight& get_instance();

			[[nodiscard]] LEOPPHAPI Vector3 const& get_intensity() const;
			LEOPPHAPI void set_intensity(Vector3 const& intensity);

		private:
			AmbientLight() = default;
		public:
			AmbientLight(AmbientLight const& other) = delete;
			AmbientLight& operator=(AmbientLight const& other) = delete;

		private:
			~AmbientLight() = default;

		public:
			AmbientLight(AmbientLight&& other) = delete;
			AmbientLight& operator=(AmbientLight&& other) = delete;

		private:
			Vector3 mIntensity{0.1f, 0.1f, 0.1f};
	};


	class DirectionalLight final : public Light
	{
		public:
			[[nodiscard]] LEOPPHAPI Vector3 const& get_direction() const;

			[[nodiscard]] LEOPPHAPI f32 get_shadow_near_plane() const;
			LEOPPHAPI void set_shadow_near_plane(f32 newVal);

			LEOPPHAPI DirectionalLight();
			DirectionalLight(DirectionalLight const& other) = delete;
			DirectionalLight(DirectionalLight&& other) = delete;

			LEOPPHAPI ~DirectionalLight() override;

			void operator=(DirectionalLight const& other) = delete;
			void operator=(DirectionalLight&& other) = delete;

		private:
			f32 mShadowNear{50.f};
	};


	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI PointLight();
			PointLight(PointLight const& other) = delete;
			PointLight(PointLight&& other) = delete;

			LEOPPHAPI ~PointLight() override;

			void operator=(PointLight const& other) = delete;
			void operator=(PointLight&& other) = delete;
	};


	class SpotLight final : public AttenuatedLight
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_inner_angle() const;
			LEOPPHAPI void set_inner_angle(f32 degrees);

			[[nodiscard]] LEOPPHAPI f32 get_outer_angle() const;
			LEOPPHAPI void set_outer_angle(f32 degrees);

			LEOPPHAPI SpotLight();
			SpotLight(SpotLight const& other) = delete;
			SpotLight(SpotLight&& other) = delete;

			LEOPPHAPI ~SpotLight() override;

			void operator=(SpotLight const& other) = delete;
			void operator=(SpotLight&& other) = delete;

		private:
			f32 mInnerAngle{30.f};
			f32 mOuterAngle{30.f};
	};
}
