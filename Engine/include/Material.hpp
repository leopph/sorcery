#pragma once

#include "Color.hpp"
#include "GlTexture.hpp"
#include "Types.hpp"
#include "Util.hpp"

#include <memory>


namespace leopph
{
	class Material
	{
		public:
			[[nodiscard]] bool is_transparent() const;
			void set_transparent(bool transparent);

		protected:
			Material() = default;

		private:
			bool mIsTransparent{false};
	};


	class BuiltInMaterial : public Material
	{
		public:
			[[nodiscard]] bool is_two_sided() const;
			void set_two_sided(bool twoSided);

			[[nodiscard]] f32 get_alpha_treshold() const;
			void set_alpha_threshold(f32 threshold);

		protected:
			BuiltInMaterial() = default;

		private:
			bool mIsTwoSided{true};
			f32 mAlphaThreshold{0.f};
	};



	class BlinnPhongMaterial : public BuiltInMaterial
	{
		public:
			Color baseColor{};
			Color SpecularColor{0, 0, 0, 0};
			f32 Gloss{1};

			std::shared_ptr<GlTexture> baseMap;
			std::shared_ptr<GlTexture> SpecularMap;
	};
}
