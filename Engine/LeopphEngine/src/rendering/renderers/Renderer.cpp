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
	auto Renderer::Create() -> std::unique_ptr<Renderer>
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



	auto Renderer::Render() -> void
	{
		ExtractMainCamera();

		if (!m_MainCamera)
		{
			return;
		}

		ExtractAllConfig();
		ExtractLights();

		SelectNearestLights<SpotLight>(m_SpotLights, (*m_MainCamera)->Owner()->Transform()->Position(), m_NumMaxSpot);
		SelectNearestLights<PointLight>(m_PointLights, (*m_MainCamera)->Owner()->Transform()->Position(), m_NumMaxPoint);
		SeparateCastingLights<SpotLight>(m_SpotLights, m_CastingSpotLights, m_NonCastingSpotLights);
		SeparateCastingLights<PointLight>(m_PointLights, m_CastingPointLights, m_NonCastingPointLights);

		UpdateDependantResources();
	}



	auto Renderer::GetRenderRes() const -> Extent2D const&
	{
		return m_RenderRes;
	}



	auto Renderer::GetDirShadowRes() const -> std::span<u16 const>
	{
		return m_DirShadowRes;
	}



	auto Renderer::GetDirCorrection() const -> f32
	{
		return m_DirCorr;
	}



	auto Renderer::GetSpotShadowRes() const -> u16
	{
		return m_SpotShadowRes;
	}



	auto Renderer::GetNumMaxSpotLights() const -> u8
	{
		return m_NumMaxSpot;
	}



	auto Renderer::GetPointShadowRes() const -> u16
	{
		return m_PointShadowRes;
	}



	auto Renderer::GetNumMaxPointLights() const -> u8
	{
		return m_NumMaxPoint;
	}



	auto Renderer::GetGamma() const -> f32
	{
		return m_Gamma;
	}



	auto Renderer::GetAmbLight() const -> Vector3 const&
	{
		return m_AmbLight;
	}



	auto Renderer::GetDirLight() const -> std::optional<DirectionalLight const*> const&
	{
		return m_DirLight;
	}



	auto Renderer::GetCastingSpotLights() const -> std::span<SpotLight const* const>
	{
		return m_CastingSpotLights;
	}



	auto Renderer::GetNonCastingSpotLights() const -> std::span<SpotLight const* const>
	{
		return m_NonCastingSpotLights;
	}



	auto Renderer::GetCastingPointLights() const -> std::span<PointLight const* const>
	{
		return m_CastingPointLights;
	}



	auto Renderer::GetNonCastingPointLights() const -> std::span<PointLight const* const>
	{
		return m_NonCastingPointLights;
	}



	auto Renderer::GetMainCamera() const -> std::optional<Camera const*> const&
	{
		return m_MainCamera;
	}



	auto Renderer::ExtractRenderRes() -> void
	{
		auto const mul = GetWindowImpl()->RenderMultiplier();
		m_RenderRes.Width = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Width()) / mul);
		m_RenderRes.Height = static_cast<u32>(static_cast<f32>(GetWindowImpl()->Height()) / mul);
	}



	auto Renderer::ExtractDirShadowRes() -> void
	{
		auto const res = GetSettingsImpl()->DirShadowResolution();
		m_DirShadowRes.assign(std::begin(res), std::end(res));
	}



	auto Renderer::ExtractSpotShadowRes() -> void
	{
		m_SpotShadowRes = GetSettingsImpl()->SpotShadowResolution();
	}



	auto Renderer::ExtractPointShadowRes() -> void
	{
		m_PointShadowRes = GetSettingsImpl()->PointShadowResolution();
	}



	auto Renderer::ExtractDirCorrection() -> void
	{
		m_DirCorr = GetSettingsImpl()->DirShadowCascadeCorrection();
	}



	auto Renderer::ExtractNumMaxSpotLights() -> void
	{
		m_NumMaxSpot = GetSettingsImpl()->MaxSpotLightCount();
	}



	auto Renderer::ExtractNumMaxPointLights() -> void
	{
		m_NumMaxPoint = GetSettingsImpl()->MaxPointLightCount();
	}



	auto Renderer::ExtractGamma() -> void
	{
		m_Gamma = GetSettingsImpl()->Gamma();
	}



	auto Renderer::ExtractAllConfig() -> void
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



	auto Renderer::ExtractLights() -> void
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



	auto Renderer::ExtractMainCamera() -> void
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



	auto Renderer::UpdateDependantResources() -> void
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



	auto Renderer::OnEventReceived(EventReceiver<WindowEvent>::EventParamType) -> void
	{
		m_ResUpdateFlags.RenderRes = true;
	}



	auto Renderer::OnEventReceived(EventReceiver<DirShadowResEvent>::EventParamType) -> void
	{
		m_ResUpdateFlags.DirShadowRes = true;
	}



	auto Renderer::OnEventReceived(EventReceiver<SpotShadowResEvent>::EventParamType) -> void
	{
		m_ResUpdateFlags.SpotShadowRes = true;
	}



	auto Renderer::OnEventReceived(EventReceiver<PointShadowResEvent>::EventParamType) -> void
	{
		m_ResUpdateFlags.PointShadowRes = true;
	}



	auto Renderer::CalculateShadowCascades(std::vector<ShadowCascade>& out) -> void
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
