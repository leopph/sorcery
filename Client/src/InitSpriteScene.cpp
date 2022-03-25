#include "AnimatedSprite.hpp"
#include "Constants.hpp"
#include "Init.hpp"
#include "controllers/CharacterController2D.hpp"
#include "controllers/FirstPersonCameraController.hpp"
#include "controllers/Follow2DCameraController.hpp"
#include "controllers/SmoothFollow2DCameraController.hpp"

#include <array>
#include <Leopph.hpp>
#include <string>

using leopph::AmbientLight;
using leopph::Entity;
using leopph::ImageSprite;
using leopph::OrthographicCamera;
using leopph::PerspectiveCamera;
using leopph::Space;
using leopph::Vector2;
using leopph::Vector3;


namespace demo
{
	auto InitSpriteScene(SceneSwitcher::Scene scene) -> void
	{
		AmbientLight::Instance().Intensity(Vector3{1});

		auto const camEntity = Entity::Find(CAM_ENTITY_NAME);
		camEntity->Transform()->Position(Vector3{0});
		camEntity->GetComponent<PerspectiveCamera>()->Deactivate();
		camEntity->GetComponent<FirstPersonCameraController>()->Deactivate();
		auto const cam = camEntity->GetComponent<OrthographicCamera>();
		cam->Activate();
		cam->MakeCurrent();
		cam->Size(5);

		std::array<std::shared_ptr<ImageSprite>, 4> sprites;
		std::string static const spritePathPrefix{"sprites/demon/demon"};
		std::string static const spritePathExt{".png"};
		for (auto i = 0; i < sprites.size(); i++)
		{
			auto static constexpr ppi = 1024;
			sprites[i] = std::make_shared<ImageSprite>(spritePathPrefix + std::to_string(i) + spritePathExt, ppi);
		}

		auto const demon = new Entity;
		demon->Transform()->Position(Vector3{0, 0, 1});
		demon->CreateAndAttachComponent<CharacterController2D>(demon->Transform(), CHAR_2D_SPEED, CHAR_2D_RUN_MULT, CHAR_2D_WALK_MULT);
		demon->CreateAndAttachComponent<AnimatedSprite>(sprites, AnimatedSprite::AnimationMode::Bounce, 2.f);

		camEntity->CreateAndAttachComponent<SmoothFollow2DCameraController>(cam.get(), demon->Transform(), Vector2{0}, 4.f);

		auto const background = new Entity;
		background->Transform()->Position(Vector3{0, 0, 2});
		background->CreateAndAttachComponent<ImageSprite>("sprites/world/ColorFlowBackground.png", 1200);
	}
}
