#include "rendering/renderers/Renderer.hpp"

#include "AmbientLight.hpp"
#include "InternalContext.hpp"
#include "Logger.hpp"
#include "SettingsImpl.hpp"
#include "../../../include/DataManager.hpp"
#include "rendering/renderers/GlRenderer.hpp"
#include "windowing/WindowImpl.hpp"

#include <stdexcept>


namespace leopph::internal
{
	std::unique_ptr<Renderer> Renderer::Create()
	{
		switch (GetSettingsImpl()->GetGraphicsApi())
		{
			case Settings::GraphicsApi::OpenGl:
				return GlRenderer::Create();
		}

		auto const errMsg{"Failed to create renderer: the selected graphics API is not supported."};
		Logger::Instance().Critical(errMsg);
		throw std::domain_error{errMsg};
	}



	void Renderer::Render()
	{
		ExtractMainCamera();

		if (!m_MainCamera)
		{
			return;
		}

		ExtractAllConfig();
		ExtractLights();

		SelectNearestLights<SpotLight>(m_SpotLights, (*m_MainCamera)->Owner()->get_transform().get_position(), m_NumMaxSpot);
		SelectNearestLights<PointLight>(m_PointLights, (*m_MainCamera)->Owner()->get_transform().get_position(), m_NumMaxPoint);
		SeparateCastingLights<SpotLight>(m_SpotLights, m_CastingSpotLights, m_NonCastingSpotLights);
		SeparateCastingLights<PointLight>(m_PointLights, m_CastingPointLights, m_NonCastingPointLights);

		UpdateDependantResources();
	}



	Extent2D const& Renderer::GetRenderRes() const
	{
		return m_RenderRes;
	}



	std::span<u16 const> Renderer::GetDirShadowRes() const
	{
		return m_DirShadowRes;
	}



	f32 Renderer::GetDirCorrection() const
	{
		return m_DirCorr;
	}



	u16 Renderer::GetSpotShadowRes() const
	{
		return m_SpotShadowRes;
	}



	u8 Renderer::GetNumMaxSpotLights() const
	{
		return m_NumMaxSpot;
	}



	u16 Renderer::GetPointShadowRes() const
	{
		return m_PointShadowRes;
	}



	u8 Renderer::GetNumMaxPointLights() const
	{
		return m_NumMaxPoint;
	}



	f32 Renderer::GetGamma() const
	{
		return m_Gamma;
	}



	Vector3 const& Renderer::GetAmbLight() const
	{
		return m_AmbLight;
	}



	std::optional<DirectionalLight const*> const& Renderer::GetDirLight() const
	{
		return m_DirLight;
	}



	std::span<SpotLight const* const> Renderer::GetCastingSpotLights() const
	{
		return m_CastingSpotLights;
	}



	std::span<SpotLight const* const> Renderer::GetNonCastingSpotLights() const
	{
		return m_NonCastingSpotLights;
	}



	std::span<PointLight const* const> Renderer::GetCastingPointLights() const
	{
		return m_CastingPointLights;
	}



	std::span<PointLight const* const> Renderer::GetNonCastingPointLights() const
	{
		return m_NonCastingPointLights;
	}



	std::optional<Camera const*> const& Renderer::GetMainCamera() const
	{
		return m_MainCamera;
	}



	void Renderer::ExtractRenderRes()
	{
		auto const mul = GetWindowImpl()->RenderMultiplier();
		m_RenderRes.Width = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Width()) / mul);
		m_RenderRes.Height = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Height()) / mul);
	}



	void Renderer::ExtractDirShadowRes()
	{
		auto const res = GetSettingsImpl()->DirShadowResolution();
		m_DirShadowRes.assign(std::begin(res), std::end(res));
	}



	void Renderer::ExtractSpotShadowRes()
	{
		m_SpotShadowRes = GetSettingsImpl()->SpotShadowResolution();
	}



	void Renderer::ExtractPointShadowRes()
	{
		m_PointShadowRes = GetSettingsImpl()->PointShadowResolution();
	}



	void Renderer::ExtractDirCorrection()
	{
		m_DirCorr = GetSettingsImpl()->DirShadowCascadeCorrection();
	}



	void Renderer::ExtractNumMaxSpotLights()
	{
		m_NumMaxSpot = GetSettingsImpl()->MaxSpotLightCount();
	}



	void Renderer::ExtractNumMaxPointLights()
	{
		m_NumMaxPoint = GetSettingsImpl()->MaxPointLightCount();
	}



	void Renderer::ExtractGamma()
	{
		m_Gamma = GetSettingsImpl()->Gamma();
	}



	void Renderer::ExtractAllConfig()
	{
		ExtractRenderRes();
		ExtractDirShadowRes();
		ExtractDirCorrection();
		ExtractSpotShadowRes();
		ExtractNumMaxSpotLights();
		ExtractPointShadowRes();
		ExtractNumMaxPointLights();
		ExtractGamma();
	}



	void Renderer::ExtractLights()
	{
		m_AmbLight = AmbientLight::Instance().Intensity();

		if (auto const* const dirLight = GetDataManager()->DirectionalLight())
		{
			m_DirLight = dirLight;
		}
		else
		{
			m_DirLight.reset();
		}

		auto const spotLights = GetDataManager()->ActiveSpotLights();
		m_SpotLights.assign(std::begin(spotLights), std::end(spotLights));

		auto const pointLights = GetDataManager()->ActivePointLights();
		m_PointLights.assign(std::begin(pointLights), std::end(pointLights));
	}



	void Renderer::ExtractMainCamera()
	{
		if (auto const* const cam = Camera::Current())
		{
			m_MainCamera = cam;
		}
		else
		{
			m_MainCamera.reset();
		}
	}



	void Renderer::UpdateDependantResources()
	{
		if (m_ResUpdateFlags.RenderRes)
		{
			ExtractRenderRes();
			OnRenderResChange(GetRenderRes());
			m_ResUpdateFlags.RenderRes = false;
		}

		if (m_ResUpdateFlags.DirShadowRes)
		{
			OnDirShadowResChange(GetSettingsImpl()->DirShadowResolution());
			m_ResUpdateFlags.DirShadowRes = false;
		}

		if (m_ResUpdateFlags.SpotShadowRes)
		{
			OnSpotShadowResChange(GetSettingsImpl()->SpotShadowResolution());
			m_ResUpdateFlags.SpotShadowRes = false;
		}

		if (m_ResUpdateFlags.PointShadowRes)
		{
			OnPointShadowResChange(GetSettingsImpl()->PointShadowResolution());
			m_ResUpdateFlags.PointShadowRes = false;
		}

		OnDetermineShadowMapCountRequirements(static_cast<u8>(m_CastingSpotLights.size()), static_cast<u8>(m_CastingPointLights.size()));
	}



	void Renderer::OnEventReceived(EventReceiver<WindowEvent>::EventParamType)
	{
		m_ResUpdateFlags.RenderRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType)
	{
		m_ResUpdateFlags.DirShadowRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType)
	{
		m_ResUpdateFlags.SpotShadowRes = true;
	}



	void Renderer::OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType)
	{
		m_ResUpdateFlags.PointShadowRes = true;
	}



	void Renderer::CalculateShadowCascades(std::vector<ShadowCascade>& out)
	{
		auto const frust = (*m_MainCamera)->Frustum();
		auto const& frustNearZ = frust.NearBottomLeft[2];
		auto const& frustFarZ = frust.FarBottomLeft[2];
		auto const frustDepth = frustFarZ - frustNearZ;
		auto const numCascades = static_cast<u8>(m_DirShadowRes.size());
		auto const lightViewMat = Matrix4::LookAt(Vector3{0}, (*m_DirLight)->Direction(), Vector3::Up());
		auto const camViewToLightViewMat = (*m_MainCamera)->ViewMatrix().Inverse() * lightViewMat;

		out.resize(numCascades);

		// Calculate the cascade near and far planes on the Z axis in main camera space

		out[0].Near = frustNearZ;
		for (auto i = 1; i < numCascades; i++)
		{
			out[i].Near = m_DirCorr * frustNearZ * math::Pow(frustFarZ / frustNearZ, static_cast<f32>(i) / static_cast<f32>(numCascades)) + (1 - m_DirCorr) * (frustNearZ + static_cast<f32>(i) / static_cast<f32>(numCascades) * (frustFarZ - frustNearZ));
			// On bound borders the far plane is multiplied by this value to avoid precision problems.
			auto constexpr borderCorrection = 1.005f;
			out[i - 1].Far = out[i].Near * borderCorrection;
		}
		out[numCascades - 1].Far = frustFarZ;

		for (auto& cascade : out)
		{
			// The normalized distance of the cascades faces from the near frustum face.

			auto const cascadeNearDist = (cascade.Near - frustNearZ) / frustDepth;
			auto const cascadeFarDist = (cascade.Far - frustNearZ) / frustDepth;

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
			auto const lightProjMat{Matrix4::Ortographic(bBoxMinLight[0], bBoxMaxLight[0], bBoxMaxLight[1], bBoxMinLight[1], bBoxMinLight[2] - (*m_DirLight)->ShadowExtension(), bBoxMaxLight[2])};

			cascade.WorldToClip = lightViewMat * lightProjMat;
		}
	}



	Renderer::Renderer()
	{
		ExtractAllConfig();
	}
}
