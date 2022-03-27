#include "Init.hpp"
#include "../AnimatedSprite.hpp"
#include "../Constants.hpp"
#include "../Parallaxer.hpp"
#include "../controllers/CharacterController2D.hpp"
#include "../controllers/FirstPersonCameraController.hpp"
#include "../controllers/Follow2DCameraController.hpp"
#include "../controllers/SmoothFollow2DCameraController.hpp"

#include <Leopph.hpp>
#include <string>
#include <vector>

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
		auto const cam = camEntity->CreateAndAttachComponent<OrthographicCamera>();
		cam->Activate();
		cam->MakeCurrent();
		cam->Size(5, leopph::Camera::Side::Vertical);
		cam->NearClipPlane(0);
		cam->FarClipPlane(10);

		std::array<std::shared_ptr<ImageSprite>, 4> demonSprites;
		std::string static const spritePathPrefix{"sprites/demon/demon"};
		std::string static const spritePathExt{".png"};
		for (auto i = 0; i < demonSprites.size(); i++)
		{
			auto static constexpr ppi = 1024;
			demonSprites[i] = leopph::CreateComponent<ImageSprite>(spritePathPrefix + std::to_string(i) += spritePathExt, ppi);
		}

		auto const demon = new Entity;
		demon->CreateAndAttachComponent<CharacterController2D>(demon->Transform(), CHAR_2D_SPEED, CHAR_2D_RUN_MULT, CHAR_2D_WALK_MULT);
		auto const animSprite = demon->CreateAndAttachComponent<AnimatedSprite>(demonSprites, AnimatedSprite::AnimationMode::Bounce, 2.f);

		auto const demon1 = new Entity;
		auto const animSpriteCopy = demon1->CreateAndAttachComponent<AnimatedSprite>(*animSprite);

		auto const followCam = camEntity->CreateAndAttachComponent<Follow2DCameraController>(cam, demon->Transform(), Vector2{0});
		followCam->UpdateIndex(1);

		auto const background = new Entity;
		background->Transform()->Position(Vector3{0, 0, 10});
		background->CreateAndAttachComponent<ImageSprite>("sprites/world/ColorFlowBackground.png", 600);

		auto const sun = new Entity;
		sun->Transform()->Position(Vector3{0, 0, 9.5});
		sun->CreateAndAttachComponent<ImageSprite>("sprites/World/Sun.png", 1024);

		auto const pinkMountains = new Entity;
		pinkMountains->Transform()->Position(Vector3{0, 1, 9});
		pinkMountains->CreateAndAttachComponent<ImageSprite>("sprites/world/PinkMountains.png", 700);

		auto const purpleMountains = new Entity;
		purpleMountains->Transform()->Position(Vector3{0, 0.5, 8});
		purpleMountains->CreateAndAttachComponent<ImageSprite>("sprites/world/PurpleMountains2.png", 225);

		auto const forest = new Entity;
		forest->Transform()->Position(Vector3{0, -0.75, 7});
		forest->CreateAndAttachComponent<ImageSprite>("sprites/world/BackgroundForest1.png", 614);

		std::vector<Parallaxer::Layer> layers
		{
			{1, background->Transform()},
			{1, sun->Transform()},
			{0.9f, pinkMountains->Transform()},
			{0.8f, purpleMountains->Transform()},
			{0.7f, forest->Transform()}
		};
		//auto const parallaxer = (new Entity{})->CreateAndAttachComponent<Parallaxer>(cam, layers);
		//parallaxer->UpdateIndex(2);
	}
}
