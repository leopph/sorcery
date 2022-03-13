#include "Constants.hpp"
#include "Init.hpp"
#include "TeleportGate.hpp"
#include "behaviors/Flicker.hpp"

#include <Leopph.hpp>

using leopph::AmbientLight;
using leopph::Vector3;
using leopph::Space;
using leopph::DirectionalLight;
using leopph::Entity;
using leopph::Model;
using leopph::PointLight;
using leopph::Skybox;


namespace demo
{
	auto InitChurchScene(SceneSwitcher::Scene thisScene, const SceneSwitcher::Scene nextScene) -> void
	{
		// Locate existing Entities
		const auto player = Entity::FindEntity(PLAYER_ENTITY_NAME);

		// Create new Entities

		const auto group = new Entity{};
		const auto dirLightEntity = new Entity{"dirlight"};
		const auto pLightEntity = new Entity{"plight"};
		const auto church = new Entity{"church"};
		const auto lamp = new Entity{"lamp"};
		const auto teleportPoint = new Entity{"churchSceneTpPoint"};

		// Add new Entities to scene vector

		thisScene.Add(group);
		thisScene.Add(dirLightEntity);
		thisScene.Add(church);
		thisScene.Add(lamp);
		thisScene.Add(pLightEntity);
		thisScene.Add(teleportPoint);

		// Set parental relations

		dirLightEntity->Transform()->Parent(group);
		pLightEntity->Transform()->Parent(lamp);
		church->Transform()->Parent(group);
		lamp->Transform()->Parent(group);
		teleportPoint->Transform()->Parent(group);

		// Set Transforms
		player->Transform()->Rotate(Vector3::Up(), 90);
		group->Transform()->Rotate(Vector3::Up(), 90, Space::World);
		dirLightEntity->Transform()->Rotate(Vector3::Up(), 45, Space::World);
		dirLightEntity->Transform()->Rotate(Vector3::Right(), 30, Space::Local);
		church->Transform()->Translate(0, -3, 0, Space::World);
		church->Transform()->Rotate(Vector3::Right(), 90, Space::Local);
		church->Transform()->Rotate(Vector3::Up(), -90, Space::World);
		lamp->Transform()->Translate(0, -1.25, 0);
		lamp->Transform()->Rotate(Vector3::Up(), -90);
		lamp->Transform()->Rescale(0.01f, 0.01f, 0.01f);
		pLightEntity->Transform()->Translate(-0.5, 3.25, -0.5, Space::Local);
		teleportPoint->Transform()->Translate(6, 0, 6.5, Space::Local);

		// Find existing components

		const auto camera = player->GetComponent<leopph::Camera>();
		const auto teleport = Entity::FindEntity(demo::TELEPORT_ENTITY_NAME)->GetComponent<TeleportGate>();

		// Create new Components

		const auto dirLight = dirLightEntity->CreateAndAttachComponent<DirectionalLight>();
		const auto pLight = pLightEntity->CreateAndAttachComponent<PointLight>();
		const auto churchModel = church->CreateAndAttachComponent<Model>("models/church/ChristchurchGreyfriarsRuinGarden03.obj");
		const auto lampModel = lamp->CreateAndAttachComponent<Model>("models/lamp/scene.gltf");
		pLightEntity->CreateAndAttachComponent<demo::Flicker>(pLight, 1.2f, 0.05f);

		// Set up components

		camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"});
		camera->NearClipPlane(0.3f);
		camera->FarClipPlane(100);
		dirLight->Diffuse(Vector3{0.1, 0.1, 0.1});
		dirLight->CastsShadow(true);
		churchModel->CastsShadow(true);
		lampModel->CastsShadow(true);
		pLight->Range(30);
		pLight->Constant(0.5f);
		pLight->Linear(0.2f);
		pLight->Quadratic(0.07f);
		pLight->CastsShadow(true);
		teleport->AddPortPoint(TeleportGate::PortPoint{teleportPoint->Transform(), 1.f, nextScene});

		// Etc

		AmbientLight::Instance().Intensity(Vector3{0});
	}
}
