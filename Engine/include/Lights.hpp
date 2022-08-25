#pragma once

#include "Math.hpp"
#include "Node.hpp"
#include "Types.hpp"


namespace leopph
{
	class Light : public Node
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
			Light(Light const& other) = default;
			Light(Light&& other) noexcept = default;

		public:
			~Light() override = default;

		protected:
			Light& operator=(Light const& other) = default;
			Light& operator=(Light&& other) noexcept = default;

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
			AttenuatedLight(AttenuatedLight const& other) = default;
			AttenuatedLight(AttenuatedLight&& other) noexcept = default;

		public:
			~AttenuatedLight() override = default;

		protected:
			AttenuatedLight& operator=(AttenuatedLight const& other) = default;
			AttenuatedLight& operator=(AttenuatedLight&& other) noexcept = default;

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
			AmbientLight(AmbientLight&& other) = delete;

		private:
			~AmbientLight() = default;

		public:
			AmbientLight& operator=(AmbientLight const& other) = delete;
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
			LEOPPHAPI DirectionalLight(DirectionalLight const& other);
			LEOPPHAPI DirectionalLight(DirectionalLight&& other) noexcept;

			LEOPPHAPI ~DirectionalLight() override;

			LEOPPHAPI DirectionalLight& operator=(DirectionalLight const& other);
			LEOPPHAPI DirectionalLight& operator=(DirectionalLight&& other) noexcept;

		private:
			f32 mShadowNear{50.f};
	};


	class PointLight final : public AttenuatedLight
	{
		public:
			LEOPPHAPI PointLight();
			LEOPPHAPI PointLight(PointLight const& other);
			LEOPPHAPI PointLight(PointLight&& other) noexcept;

			LEOPPHAPI ~PointLight() override;

			LEOPPHAPI PointLight& operator=(PointLight const& other);
			LEOPPHAPI PointLight& operator=(PointLight&& other) noexcept;
	};


	class SpotLight final : public AttenuatedLight
	{
		public:
			[[nodiscard]] LEOPPHAPI f32 get_inner_angle() const;
			LEOPPHAPI void set_inner_angle(f32 degrees);

			[[nodiscard]] LEOPPHAPI f32 get_outer_angle() const;
			LEOPPHAPI void set_outer_angle(f32 degrees);

			LEOPPHAPI SpotLight();
			LEOPPHAPI SpotLight(SpotLight const& other);
			LEOPPHAPI SpotLight(SpotLight&& other) noexcept;

			LEOPPHAPI ~SpotLight() override;

			LEOPPHAPI SpotLight& operator=(SpotLight const& other);
			LEOPPHAPI SpotLight& operator=(SpotLight&& other) noexcept;

		private:
			f32 mInnerAngle{30.f};
			f32 mOuterAngle{30.f};
	};
}
