#include "AnimatedSprite.hpp"
#include "Constants.hpp"
#include "Init.hpp"
#include "Parallaxer.hpp"
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
		cam->Size(10);
		cam->NearClipPlane(0);
		cam->FarClipPlane(10);

		std::array<std::shared_ptr<ImageSprite>, 4> sprites;
		std::string static const spritePathPrefix{"sprites/demon/demon"};
		std::string static const spritePathExt{".png"};
		for (auto i = 0; i < sprites.size(); i++)
		{
			auto static constexpr ppi = 512;
			sprites[i] = leopph::CreateComponent<ImageSprite>(spritePathPrefix + std::to_string(i) += spritePathExt, ppi);
		}


		auto const demon = new Entity;
		demon->CreateAndAttachComponent<CharacterController2D>(demon->Transform(), CHAR_2D_SPEED, CHAR_2D_RUN_MULT, CHAR_2D_WALK_MULT);
		demon->CreateAndAttachComponent<AnimatedSprite>(sprites, AnimatedSprite::AnimationMode::Bounce, 2);

		camEntity->CreateAndAttachComponent<Follow2DCameraController>(cam, demon->Transform(), Vector2{0});

		auto const background = new Entity;
		background->Transform()->Position(Vector3{0, 0, 10});
		background->CreateAndAttachComponent<ImageSprite>("sprites/world/ColorFlowBackground.png", 600);


		std::array arr{Parallaxer::Layer{-0.5, background->Transform()}};
		auto const parallaxer = (new Entity{})->CreateAndAttachComponent<Parallaxer>(cam, arr);
	}
}
