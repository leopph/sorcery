#include "Renderer.hpp"

#include "AmbientLight.hpp"
#include "GlRenderer.hpp"
#include "Logger.hpp"
#include "../InternalContext.hpp"
#include "../SettingsImpl.hpp"
#include "../data/DataManager.hpp"
#include "../windowing/WindowImpl.hpp"


namespace leopph::internal
{
	void Renderer::render()
	{
		if (!extract())
		{
			return;
		}

		prepare();
		draw();

		if (!mMainCam)
		{
			return;
		}

		extract_all_config();
		extract_all_lights();

		select_nearest_lights<SpotLight>(mSpotLights, (*mMainCam)->Owner()->get_transform().get_position(), mNumMaxSpot);
		select_nearest_lights<PointLight>(mPointLights, (*mMainCam)->Owner()->get_transform().get_position(), mNumMaxPoint);
		separate_casting_lights<SpotLight>(mSpotLights, mCastingSpots, mNonCastingSpots);
		separate_casting_lights<PointLight>(mPointLights, mCastingPoints, mNonCastingPoints);

		update_dependant_resources();
	}



	void Renderer::extract_spot_shadow_res()
	{
		mSpotShadowRes = GetSettingsImpl()->SpotShadowResolution();
	}



	void Renderer::extract_point_shadow_res()
	{
		mPointShadowRes = GetSettingsImpl()->PointShadowResolution();
	}



	void Renderer::extract_num_max_spot_lights()
	{
		mNumMaxSpot = GetSettingsImpl()->MaxSpotLightCount();
	}



	void Renderer::extract_num_max_point_lights()
	{
		mNumMaxPoint = GetSettingsImpl()->MaxPointLightCount();
	}



	void Renderer::extract_all_config()
	{
		extract_spot_shadow_res();
		extract_num_max_spot_lights();
		extract_point_shadow_res();
		extract_num_max_point_lights();
	}



	void Renderer::extract_all_lights()
	{
		mAmbLight = AmbientLight::Instance().Intensity();

		auto const spotLights = GetDataManager()->ActiveSpotLights();
		mSpotLights.assign(std::begin(spotLights), std::end(spotLights));

		auto const pointLights = GetDataManager()->ActivePointLights();
		mPointLights.assign(std::begin(pointLights), std::end(pointLights));
	}



	bool Renderer::extract()
	{
		// Extract main camera

		auto const* const cam = Camera::Current();

		// If there is no camera, abort extract and signal abort for the parent process too
		if (!cam)
		{
			return false;
		}

		mCamData.pos = cam->Owner()->get_transform().get_position();
		mCamData.transformData.viewMat = cam->ViewMatrix();
		mCamData.transformData.viewMatInv = mCamData.transformData.viewMat.Inverse();
		mCamData.transformData.projMat = cam->ProjectionMatrix();
		mCamData.transformData.projMatInv = mCamData.transformData.projMat.Inverse();
		mCamData.transformData.viewProjMat = mCamData.transformData.viewMat * mCamData.transformData.projMat;
		mCamData.transformData.viewProjMatInv = mCamData.transformData.viewProjMat.Inverse();

		// Extract screen data

		auto const renderMult = GetWindowImpl()->RenderMultiplier();
		mScreenData.width = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Width()) / renderMult);
		mScreenData.height = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Height()) / renderMult);
		mScreenData.gamma = GetSettingsImpl()->Gamma();

		// Extract directional light data
		if (auto const* const dirLight = GetDataManager()->DirectionalLight())
		{
			DirLightData dirLightData;
			dirLightData.direction = dirLight->Direction();
			dirLightData.diffuse = dirLight->Diffuse();
			dirLightData.specular = dirLight->Specular();

			if (dirLight->CastsShadow())
			{
				ShadowCascadeData cascadeData;
				cascadeData.correction = GetSettingsImpl()->DirShadowCascadeCorrection();
				cascadeData.nearClip = dirLight->ShadowExtension();
				std::ranges::copy(GetSettingsImpl()->DirShadowResolution(), std::back_inserter(cascadeData.res));
				dirLightData.cascades = std::move(cascadeData);
			}

			mDirData = std::move(dirLightData);
		}
		else
		{
			mDirData.reset();
		}

		extract_all_config();
		extract_all_lights();

		return true;
	}



	void Renderer::prepare_all_lighting_data()
	{
		select_nearest_lights<SpotLight>(mSpotLights, (*mMainCam)->Owner()->get_transform().get_position(), mNumMaxSpot);
		select_nearest_lights<PointLight>(mPointLights, (*mMainCam)->Owner()->get_transform().get_position(), mNumMaxPoint);

		separate_casting_lights<SpotLight>(mSpotLights, mCastingSpots, mNonCastingSpots);
		separate_casting_lights<PointLight>(mPointLights, mCastingPoints, mNonCastingPoints);
	}



	void Renderer::update_dependant_resources()
	{
		if (mResUpdateFlags.renderRes)
		{
			extract_render_res();
			on_render_res_change(mRenderRes);
			mResUpdateFlags.renderRes = false;
		}

		if (mResUpdateFlags.dirShadowRes)
		{
			on_dir_shadow_res_change(GetSettingsImpl()->DirShadowResolution());
			mResUpdateFlags.dirShadowRes = false;
		}

		if (mResUpdateFlags.spotShadowRes)
		{
			on_spot_shadow_res_change(GetSettingsImpl()->SpotShadowResolution());
			mResUpdateFlags.spotShadowRes = false;
		}

		if (mResUpdateFlags.pointShadowRes)
		{
			on_point_shadow_res_change(GetSettingsImpl()->PointShadowResolution());
			mResUpdateFlags.pointShadowRes = false;
		}

		on_determine_shadow_map_count_requirements(static_cast<u8>(mCastingSpots.size()), static_cast<u8>(mCastingPoints.size()));
	}



	void Renderer::forward_render()
	{}



	void Renderer::deferred_render()
	{}



	void Renderer::OnEventReceived(EventReceiver<WindowEvent>::EventParamType)
	{
		mResUpdateFlags.renderRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType)
	{
		mResUpdateFlags.dirShadowRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType)
	{
		mResUpdateFlags.spotShadowRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType)
	{
		mResUpdateFlags.pointShadowRes = true;
	}



	void Renderer::calculate_shadow_cascades(std::vector<ShadowCascade>& out)
	{
		auto const frust = (*mMainCam)->Frustum();
		auto const& frustNearZ = frust.NearBottomLeft[2];
		auto const& frustFarZ = frust.FarBottomLeft[2];
		auto const frustDepth = frustFarZ - frustNearZ;
		auto const numCascades = static_cast<u8>(mDirShadowRes.size());
		auto const lightViewMat = Matrix4::LookAt(Vector3{0}, (*mDirLight)->Direction(), Vector3::Up());
		auto const camViewToLightViewMat = (*mMainCam)->ViewMatrix().Inverse() * lightViewMat;

		out.resize(numCascades);

		// Calculate the cascade near and far planes on the Z axis in main camera space

		out[0].near = frustNearZ;
		for (auto i = 1; i < numCascades; i++)
		{
			out[i].near = mDirCorr * frustNearZ * math::Pow(frustFarZ / frustNearZ, static_cast<f32>(i) / static_cast<f32>(numCascades)) + (1 - mDirCorr) * (frustNearZ + static_cast<f32>(i) / static_cast<f32>(numCascades) * (frustFarZ - frustNearZ));
			// On bound borders the far plane is multiplied by this value to avoid precision problems.
			auto constexpr borderCorrection = 1.005f;
			out[i - 1].far = out[i].near * borderCorrection;
		}
		out[numCascades - 1].far = frustFarZ;

		for (auto& cascade : out)
		{
			// The normalized distance of the cascades faces from the near frustum face.

			auto const cascadeNearDist = (cascade.near - frustNearZ) / frustDepth;
			auto const cascadeFarDist = (cascade.far - frustNearZ) / frustDepth;

			// The cascade vertices in camera view space.
			std::array const cascadeVertsCam
			{
				Vector4{math::Lerp(frust.NearTopLeft, frust.FarTopLeft, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frust.NearBottomLeft, frust.FarBottomLeft, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frust.NearBottomRight, frust.FarBottomRight, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frust.NearTopRight, frust.FarTopRight, cascadeNearDist), 1.f},
				Vector4{math::Lerp(frust.NearTopLeft, frust.FarTopLeft, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frust.NearBottomLeft, frust.FarBottomLeft, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frust.NearBottomRight, frust.FarBottomRight, cascadeFarDist), 1.f},
				Vector4{math::Lerp(frust.NearTopRight, frust.FarTopRight, cascadeFarDist), 1.f},
			};

			// The light view space mininum point of the bounding box of the cascade
			Vector3 bBoxMinLight{std::numeric_limits<float>::max()};

			// The light view space maximum point of the bounding box of the cascade
			Vector3 bBoxMaxLight{std::numeric_limits<float>::min()};

			// Calculate the bounding box min and max points by transforming the vertices to light space.
			std::ranges::for_each(cascadeVertsCam, [&](auto const& vertex)
			{
				auto const vertLight = vertex * camViewToLightViewMat;
				bBoxMinLight = Vector3{std::min(bBoxMinLight[0], vertLight[0]), std::min(bBoxMinLight[1], vertLight[1]), std::min(bBoxMinLight[2], vertLight[2])};
				bBoxMaxLight = Vector3{std::max(bBoxMaxLight[0], vertLight[0]), std::max(bBoxMaxLight[1], vertLight[1]), std::max(bBoxMaxLight[2], vertLight[2])};
			});

			// The projection matrix that uses the calculated min/max values of the bounding box. Essentially THE bounding box + the near clip offset of the DirectionalLight.
			auto const lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2] - (*mDirLight)->ShadowExtension(), bBoxMaxLight[2])};

			cascade.wordToClip = lightViewMat * lightProjMat;
		}
	}



	Renderer::Renderer()
	{
		extract_all_config();
	}
}
