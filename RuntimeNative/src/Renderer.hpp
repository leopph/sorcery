#pragma once

#include "Core.hpp"
#include "StaticMeshComponent.hpp"
#include "LightComponents.hpp"
#include "Util.hpp"
#include "SkyboxComponent.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <d3d11.h>


namespace leopph::renderer {
class Camera {
public:
	enum class Type : std::uint8_t {
		Perspective  = 0,
		Orthographic = 1
	};


	enum class Side : std::uint8_t {
		Vertical   = 0,
		Horizontal = 1
	};


	[[nodiscard]] virtual auto GetPosition() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetForwardAxis() const noexcept -> Vector3 = 0;
	[[nodiscard]] virtual auto GetNearClipPlane() const noexcept -> float = 0;
	[[nodiscard]] virtual auto GetFarClipPlane() const noexcept -> float = 0;
	[[nodiscard]] virtual auto GetHorizontalOrthographicSize() const -> float = 0;
	[[nodiscard]] virtual auto GetHorizontalPerspectiveFov() const -> float = 0;
	[[nodiscard]] virtual auto GetType() const -> Type = 0;
	[[nodiscard]] virtual auto GetFrustum(float aspectRatio) const -> Frustum = 0;

	[[nodiscard]] static auto LEOPPHAPI HorizontalPerspectiveFovToVertical(float fovDegrees, float aspectRatio) noexcept -> float;
	[[nodiscard]] static auto LEOPPHAPI VerticalPerspectiveFovToHorizontal(float fovDegrees, float aspectRatio) noexcept -> float;

	Camera() = default;
	Camera(Camera const& other) = default;
	Camera(Camera&& other) noexcept = default;

	auto operator=(Camera const& other) -> Camera& = default;
	auto operator=(Camera&& other) noexcept -> Camera& = default;

	virtual ~Camera() = default;
};


LEOPPHAPI auto StartUp() -> void;
LEOPPHAPI auto ShutDown() noexcept -> void;

LEOPPHAPI auto DrawGame() noexcept -> void;
LEOPPHAPI auto DrawSceneView(Camera const& cam) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetGameResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSceneResolution() noexcept -> Extent2D<u32>;
LEOPPHAPI auto SetSceneResolution(Extent2D<u32> resolution) noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetGameFrame() noexcept -> ID3D11ShaderResourceView*;
LEOPPHAPI [[nodiscard]] auto GetSceneFrame() noexcept -> ID3D11ShaderResourceView*;

LEOPPHAPI [[nodiscard]] auto GetGameAspectRatio() noexcept -> f32;
LEOPPHAPI [[nodiscard]] auto GetSceneAspectRatio() noexcept -> f32;

LEOPPHAPI auto BindAndClearSwapChain() noexcept -> void;

LEOPPHAPI auto Present() noexcept -> void;

LEOPPHAPI [[nodiscard]] auto GetSyncInterval() noexcept -> u32;
LEOPPHAPI auto SetSyncInterval(u32 interval) noexcept -> void;

LEOPPHAPI auto RegisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;
LEOPPHAPI auto UnregisterStaticMesh(StaticMeshComponent const* staticMesh) -> void;

LEOPPHAPI [[nodiscard]] auto GetDevice() noexcept -> ID3D11Device*;
LEOPPHAPI [[nodiscard]] auto GetImmediateContext() noexcept -> ID3D11DeviceContext*;

LEOPPHAPI auto RegisterLight(LightComponent const* light) -> void;
LEOPPHAPI auto UnregisterLight(LightComponent const* light) -> void;

LEOPPHAPI [[nodiscard]] auto GetDefaultMaterial() noexcept -> Material*;
LEOPPHAPI [[nodiscard]] auto GetCubeMesh() noexcept -> Mesh*;
LEOPPHAPI [[nodiscard]] auto GetPlaneMesh() noexcept -> Mesh*;

LEOPPHAPI [[nodiscard]] auto GetGamma() noexcept -> f32;
LEOPPHAPI auto SetGamma(f32 gamma) noexcept -> void;

LEOPPHAPI auto RegisterSkybox(SkyboxComponent const* skybox) -> void;
LEOPPHAPI auto UnregisterSkybox(SkyboxComponent const* skybox) -> void;

LEOPPHAPI auto RegisterGameCamera(Camera const& cam) -> void;
LEOPPHAPI auto UnregisterGameCamera(Camera const& cam) -> void;
}
