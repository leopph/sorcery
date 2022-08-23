/*#include "Init.hpp"
#include "../AnimatedSprite.hpp"
#include "../Parallaxer.hpp"
#include "../Tiler.hpp"
#include "../controllers/CharacterController2D.hpp"
#include "../controllers/Follow2DCameraController.hpp"

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
	void InitSpriteScene(SceneSwitcher::Scene& scene)
	{
		auto const camEntity = new Entity;
		scene.Add(camEntity);
		camEntity->get_transform().set_position(Vector3{0});

		auto const cam = camEntity->attach_component<OrthographicCamera>();
		cam->Activate();
		cam->MakeCurrent();
		cam->Size(10, leopph::Camera::Side::Vertical);
		cam->NearClipPlane(0);
		cam->FarClipPlane(10);

		std::array<std::shared_ptr<ImageSprite>, 4> demonSprites;
		std::string static const spritePathPrefix{"sprites/demon/demon"};
		std::string static const spritePathExt{".png"};
		for (auto i = 0; i < demonSprites.size(); i++)
		{
			auto static constexpr ppi = 512;
			demonSprites[i] = leopph::CreateComponent<ImageSprite>(spritePathPrefix + std::to_string(i) += spritePathExt, ppi);
		}

		auto const demon = new Entity;
		scene.Add(demon);
		demon->attach_component<CharacterController2D>(demon->get_transform(), 3.0f, 5.0f, 0.2f);
		auto const animSprite = demon->attach_component<AnimatedSprite>(demonSprites, AnimatedSprite::AnimationMode::Bounce, 2.f);

		auto const followCam = camEntity->attach_component<Follow2DCameraController>(cam, demon->get_transform(), Vector2{0});
		followCam->UpdateIndex(1);

		auto const backgroundLayer = new Entity;
		scene.Add(backgroundLayer);
		backgroundLayer->get_transform().set_position(Vector3{0, 0, 10});

		auto const background = new Entity;
		background->get_transform().set_parent(backgroundLayer);
		background->attach_component<ImageSprite>("sprites/world/ColorFlowBackground.png", 100);

		auto const sun = new Entity;
		sun->get_transform().set_parent(backgroundLayer);
		sun->get_transform().set_position(Vector3{-1.93, 2.63, 9.5});
		sun->attach_component<ImageSprite>("sprites/World/Sun.png", 512);

		auto const farLayer = new Entity;
		scene.Add(farLayer);
		farLayer->get_transform().set_position(Vector3{0, 1.5, 9});

		auto const pinkMountains = new Entity;
		pinkMountains->get_transform().set_parent(farLayer);
		auto const pinkMSprite = pinkMountains->attach_component<ImageSprite>("sprites/world/PinkMountains.png", 384);
		pinkMSprite->Instanced(true);

		auto const midLayer = new Entity;
		scene.Add(midLayer);
		midLayer->get_transform().set_position(Vector3{0, 0.9, 8});

		auto const purpleMountains = new Entity;
		purpleMountains->get_transform().set_parent(midLayer);
		auto const purpMSprite = purpleMountains->attach_component<ImageSprite>("sprites/world/PurpleMountains2.png", 100);
		purpMSprite->Instanced(true);

		auto const nearLayer = new Entity;
		scene.Add(nearLayer);
		nearLayer->get_transform().set_position(Vector3{0, -1.92, 7});

		auto const forest = new Entity;
		forest->get_transform().set_parent(nearLayer);
		auto const forestSprite = forest->attach_component<ImageSprite>("sprites/world/BackgroundForest1.png", 256);
		forestSprite->Instanced(true);

		auto const groundLayer = new Entity;
		scene.Add(groundLayer);
		groundLayer->get_transform().set_position(Vector3{0, -4, 6});

		auto const ground = new Entity;
		ground->get_transform().set_parent(groundLayer);
		auto const groundSprite = ground->attach_component<ImageSprite>("sprites/world/Ground1.png", 512);
		groundSprite->Instanced(true);

		std::vector<Parallaxer::Layer> parallaxLayers
		{
			{1, &backgroundLayer->get_transform()},
			{0.9f, &farLayer->get_transform()},
			{0.8f, &midLayer->get_transform()},
			{0.7f, &nearLayer->get_transform()},
			{0.f, &groundLayer->get_transform()}
		};
		auto const parallaxer = (new Entity{})->attach_component<Parallaxer>(cam, parallaxLayers);
		parallaxer->UpdateIndex(3);
		scene.Add(parallaxer->get_owner());

		std::vector<Tiler::Layer> tileLayers
		{
			{pinkMountains, pinkMountains, pinkMountains},
			{purpleMountains, purpleMountains, purpleMountains},
			{forest, forest, forest},
			{ground, ground, ground}
		};
		auto const tiler = (new Entity)->attach_component<Tiler>(tileLayers);
		tiler->UpdateIndex(2);
		scene.Add(tiler->get_owner());

		scene.SetActivationCallback([cam, demon]
		{
			cam->MakeCurrent();
			demon->get_transform().set_local_position(Vector3{0});
			AmbientLight::Instance().Intensity(Vector3{1});
		});
	}
}
*/