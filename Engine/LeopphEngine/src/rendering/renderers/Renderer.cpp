#include "rendering/renderers/Renderer.hpp"

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



	auto Renderer::GetDirLight() const -> std::optional<DirectionalLight const*> const&
	{
		return m_DirLight;
	}



	auto Renderer::GetCastingSpotLights() const -> std::span<SpotLight const* const>
	{
		return m_CastingSpotLights;;
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



	Renderer::Renderer()
	{
		ExtractAllConfig();
	}
}
