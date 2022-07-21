#include "Init.hpp"
#include "../Flicker.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

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
	auto InitChurchScene(SceneSwitcher::Scene& scene) -> void
	{
		auto const group = new Entity{};
		scene.Add(group);
		group->Transform()->Rotate(Vector3::Up(), 90);

		auto const player = new Entity{};
		player->Transform()->Parent(group);

		auto const camera = player->CreateAndAttachComponent<leopph::PerspectiveCamera>();
		camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"});
		camera->NearClipPlane(0.1f);
		camera->FarClipPlane(100);

		player->CreateAndAttachComponent<demo::FirstPersonCameraController>(camera, 2.0f, 0.1f, 5.0f, 0.2f);
		auto const sLight = player->CreateAndAttachComponent<leopph::SpotLight>();
		sLight->Diffuse(Vector3{1});
		sLight->Specular(sLight->Diffuse());
		sLight->InnerAngle(25);
		sLight->OuterAngle(35);
		sLight->Range(5);

		auto const dirLightEntity = new Entity{"dirlight"};
		dirLightEntity->Transform()->Parent(group);
		dirLightEntity->Transform()->Rotate(Vector3::Up(), 45);
		dirLightEntity->Transform()->Rotate(Vector3::Right(), 30, Space::Local);

		auto const dirLight = dirLightEntity->CreateAndAttachComponent<DirectionalLight>();
		dirLight->Diffuse(Vector3{.06f});
		dirLight->Specular(dirLight->Diffuse());
		dirLight->CastsShadow(false);

		auto const lamp = new Entity{"lamp"};
		lamp->Transform()->Parent(group);
		lamp->Transform()->Translate(0, -1.25, 0);
		lamp->Transform()->Rotate(Vector3::Up(), -90);
		lamp->Transform()->Rescale(0.01f, 0.01f, 0.01f);
		
		auto const lampModel = lamp->CreateAndAttachComponent<Model>("models/lamp/lamp.leopph3d");
		lampModel->CastsShadow(true);

		auto const pLightEntity = new Entity{"plight"};
		pLightEntity->Transform()->Parent(lamp);
		pLightEntity->Transform()->Translate(-0.7f, 3.7f, 0,  Space::Local);

		auto const pLight = pLightEntity->CreateAndAttachComponent<PointLight>();
		pLight->Diffuse(Vector3{1});
		pLight->Specular(pLight->Diffuse());
		pLight->Range(7);
		pLight->CastsShadow(false);

		pLightEntity->CreateAndAttachComponent<Flicker>(pLight, 0.75f, 3.5f, 0.05f, 0.6f);

		auto const church = new Entity{"church"};
		church->Transform()->Parent(group);
		church->Transform()->Translate(0, -3, 0, Space::World);
		church->Transform()->Rotate(Vector3::Right(), 90);
		church->Transform()->Rotate(Vector3::Up(), -90);
		
		auto const churchModel = church->CreateAndAttachComponent<Model>("models/church/church.leopph3d");
		churchModel->CastsShadow(true);

		scene.SetActivationCallback([camera, player]()
		{
			camera->MakeCurrent();
			player->Transform()->LocalPosition(Vector3{1, 1, -5});
			player->Transform()->LocalRotation(leopph::Quaternion{});
			AmbientLight::Instance().Intensity(Vector3{0.01f});
		});
	}
}
