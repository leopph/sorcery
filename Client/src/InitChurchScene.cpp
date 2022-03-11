#include "Constants.hpp"
#include "Init.hpp"
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
	auto InitChurchScene(std::vector<Entity*>& createdEntities) -> void
	{
		// Locate existing Entities
		const auto player = Entity::FindEntity(PLAYER_ENTITY_NAME);

		// Create new Entities

		const auto group = new Entity{};
		const auto dirLightEntity = new Entity{"dirlight"};
		const auto church = new Entity{"church"};
		const auto lamp = new Entity{"lamp"};
		const auto pLightEntity = new Entity{"plight"};

		// Add new Entities to scene vector

		createdEntities.push_back(group);
		createdEntities.push_back(dirLightEntity);
		createdEntities.push_back(church);
		createdEntities.push_back(lamp);
		createdEntities.push_back(pLightEntity);

		// Set parental relations

		church->Transform()->Parent(group);
		lamp->Transform()->Parent(group);
		pLightEntity->Transform()->Parent(lamp);

		// Set Transforms

		group->Transform()->Translate(0, -3, 0);
		player->Transform()->Rotate(Vector3::Up(), 90);
		dirLightEntity->Transform()->Rotate(Vector3::Up(), 135, Space::World);
		dirLightEntity->Transform()->Rotate(Vector3::Right(), 45, Space::Local);
		church->Transform()->Rotate(Vector3::Right(), 90);
		lamp->Transform()->Translate(0, 1.75, 0);
		lamp->Transform()->Rescale(0.01f, 0.01f, 0.01f);
		pLightEntity->Transform()->Translate(-0.5, 3.25, -0.5, Space::Local);

		// Find existing components

		const auto camera = player->GetComponent<leopph::Camera>();

		// Create new Components

		const auto dirLight = dirLightEntity->CreateAndAttachComponent<DirectionalLight>();
		const auto pLight = pLightEntity->CreateAndAttachComponent<PointLight>();
		const auto churchModel = church->CreateAndAttachComponent<Model>("models/church/ChristchurchGreyfriarsRuinGarden03.obj");
		const auto lampModel = lamp->CreateAndAttachComponent<Model>("models/lamp/scene.gltf");
		const auto flicker = pLightEntity->CreateAndAttachComponent<demo::Flicker>(pLight, 1.2f, 0.05f);

		// Set up components

		camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"});
		camera->NearClipPlane(0.3f);
		camera->FarClipPlane(1000);
		dirLight->Diffuse(Vector3{0.07, 0.07, 0.07});
		dirLight->CastsShadow(true);
		churchModel->CastsShadow(true);
		lampModel->CastsShadow(true);
		pLight->Range(30);
		pLight->Constant(0.5f);
		pLight->Linear(0.2f);
		pLight->Quadratic(0.07f);
		pLight->CastsShadow(true);

		// Etc

		AmbientLight::Instance().Intensity(Vector3{0.05, 0.05, 0.05});
	}
}
