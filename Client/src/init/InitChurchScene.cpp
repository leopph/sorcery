#include "Init.hpp"
#include "../Flicker.hpp"
#include "../controllers/FirstPersonCameraController.hpp"

#include <Leopph.hpp>

using leopph::AmbientLight;
using leopph::Vector3;
using leopph::Space;
using leopph::DirectionalLight;
using leopph::Entity;
using leopph::StaticModelComponent;
using leopph::PointLight;
using leopph::Skybox;


namespace demo
{
	void InitChurchScene(SceneSwitcher::Scene& scene)
	{
		auto const group = new Entity{};
		scene.Add(group);
		group->get_transform().rotate(Vector3::Up(), 90);

		auto const player = new Entity{};
		player->get_transform().set_parent(group);

		auto const camera = player->create_and_attach_component<leopph::PerspectiveCamera>();
		camera->Background(Skybox{"skybox/megasun/left.hdr", "skybox/megasun/right.hdr", "skybox/megasun/top.hdr", "skybox/megasun/bottom.hdr", "skybox/megasun/front.hdr", "skybox/megasun/back.hdr"});
		camera->NearClipPlane(0.1f);
		camera->FarClipPlane(100);

		player->create_and_attach_component<FirstPersonCameraController>(camera, 2.0f, 0.1f, 5.0f, 0.2f);
		auto const sLight = player->create_and_attach_component<leopph::SpotLight>();
		sLight->set_color(Vector3{1});
		sLight->Specular(sLight->get_color());
		sLight->set_inner_angle(25);
		sLight->set_outer_angle(35);
		sLight->set_range(5);

		auto const dirLightEntity = new Entity{"dirlight"};
		dirLightEntity->get_transform().set_parent(group);
		dirLightEntity->get_transform().rotate(Vector3::Up(), 45);
		dirLightEntity->get_transform().rotate(Vector3::Right(), 30, Space::Local);

		auto const dirLight = dirLightEntity->create_and_attach_component<DirectionalLight>();
		dirLight->set_color(Vector3{.06f});
		dirLight->Specular(dirLight->get_color());
		dirLight->set_casting_shadow(false);

		auto const lamp = new Entity{"lamp"};
		lamp->get_transform().set_parent(group);
		lamp->get_transform().translate(0, -1.25, 0);
		lamp->get_transform().rotate(Vector3::Up(), -90);
		lamp->get_transform().rescale(0.01f, 0.01f, 0.01f);

		auto const lampModel = lamp->create_and_attach_component<StaticModelComponent>("models/lamp/lamp.leopph3d");
		lampModel->set_casting_shadow(true);

		auto const pLightEntity = new Entity{"plight"};
		pLightEntity->get_transform().set_parent(lamp);
		pLightEntity->get_transform().translate(-0.7f, 3.7f, 0, Space::Local);

		auto const pLight = pLightEntity->create_and_attach_component<PointLight>();
		pLight->set_color(Vector3{1});
		pLight->Specular(pLight->get_color());
		pLight->set_range(7);
		pLight->set_casting_shadow(false);

		pLightEntity->create_and_attach_component<Flicker>(pLight, 0.75f, 3.5f, 0.05f, 0.6f);

		auto const church = new Entity{"church"};
		church->get_transform().set_parent(group);
		church->get_transform().translate(0, -3, 0, Space::World);
		church->get_transform().rotate(Vector3::Right(), 90);
		church->get_transform().rotate(Vector3::Up(), -90);

		auto const churchModel = church->create_and_attach_component<StaticModelComponent>("models/church/church.leopph3d");
		churchModel->set_casting_shadow(true);

		scene.SetActivationCallback([camera, player]()
		{
			camera->MakeCurrent();
			player->get_transform().set_local_position(Vector3{1, 1, -5});
			player->get_transform().set_local_rotation(leopph::Quaternion{});
			AmbientLight::Instance().Intensity(Vector3{0.01f});
		});
	}
}
