#pragma once

#include "graphics.hpp"
#include "render_target.hpp"
#include "../Core.hpp"
#include "../Math.hpp"
#include "../observer_ptr.hpp"

#include <DirectXTex.h>

#include <array>
#include <cstddef>
#include <mutex>
#include <span>
#include <variant>
#include <vector>


namespace sorcery::rendering {
class RenderManager {
public:
  LEOPPHAPI explicit RenderManager(graphics::GraphicsDevice& device);
  RenderManager(RenderManager const&) = delete;
  RenderManager(RenderManager&&) = delete;

  LEOPPHAPI ~RenderManager();

  auto operator=(RenderManager const&) -> void = delete;
  auto operator=(RenderManager&&) -> void = delete;

  [[nodiscard]] constexpr static auto GetMaxGpuQueuedFrames() -> UINT;
  [[nodiscard]] constexpr static auto GetMaxFramesInFlight() -> UINT;

  [[nodiscard]] LEOPPHAPI auto GetCurrentFrameCount() const -> UINT64;
  [[nodiscard]] LEOPPHAPI auto GetCurrentFrameIndex() const -> UINT;
  [[nodiscard]] LEOPPHAPI auto GetPreviousFrameIndex() const -> UINT;

  [[nodiscard]] LEOPPHAPI auto AcquireCommandList() -> graphics::CommandList&;
  [[nodiscard]] LEOPPHAPI auto AcquireTemporaryRenderTarget(
    RenderTarget::Desc const& desc) -> std::shared_ptr<RenderTarget>;

  LEOPPHAPI auto UpdateBuffer(graphics::Buffer const& buf, UINT byte_offset, std::span<std::byte const> data) -> void;
  LEOPPHAPI auto UpdateTexture(graphics::Texture const& tex, UINT subresource_offset,
                               std::span<D3D12_SUBRESOURCE_DATA const> data) -> void;

  [[nodiscard]] LEOPPHAPI auto CreateReadOnlyTexture(
    DirectX::ScratchImage const& img) -> graphics::SharedDeviceChildHandle<graphics::Texture>;

  LEOPPHAPI auto KeepAliveWhileInUse(graphics::SharedDeviceChildHandle<graphics::Buffer> buf) -> void;
  LEOPPHAPI auto KeepAliveWhileInUse(graphics::SharedDeviceChildHandle<graphics::Texture> tex) -> void;

  // At the end of a frame this must be called!
  LEOPPHAPI auto EndFrame() -> void;

private:
  struct TempRenderTargetRecord {
    std::shared_ptr<RenderTarget> rt;
    UINT age_in_frames;
  };


  struct KeepAliveRecord {
    std::variant<graphics::SharedDeviceChildHandle<graphics::Buffer>, graphics::SharedDeviceChildHandle<
                   graphics::Texture>> res;
    UINT age;
  };


  auto CreateCommandLists(UINT count) -> void;
  auto AgeTempRenderTargets() -> void;
  auto AgeKeepAliveBuffers() -> void;
  auto ReleaseOldTempRenderTargets() -> void;
  auto ReleaseUnusedBuffers() -> void;
  auto RecreateUploadBuffer(UINT64 size) -> void;
  auto WaitForAllUploads() -> void;
  auto WaitForInFlightFrames() const -> void;
  auto UpdateCounters() -> void;

  static UINT constexpr max_tmp_rt_age_{10};
  static UINT constexpr max_gpu_queued_frames_{1};
  static UINT constexpr max_frames_in_flight_{max_gpu_queued_frames_ + 1};

  static_assert(
    max_tmp_rt_age_ > max_gpu_queued_frames_ &&
    "Temporary render targets must live long enough to let any work possibly queued on them finish!");

  ObserverPtr<graphics::GraphicsDevice> device_;

  UINT64 frame_count_{0};
  UINT frame_idx_{0};
  UINT next_cmd_list_idx_{0};

  std::vector<std::array<graphics::SharedDeviceChildHandle<graphics::CommandList>, max_frames_in_flight_>> cmd_lists_;
  std::mutex cmd_list_mutex_;

  std::vector<TempRenderTargetRecord> tmp_render_targets_;
  std::mutex tmp_render_targets_mutex_;

  graphics::SharedDeviceChildHandle<graphics::Fence> in_flight_frames_fence_;

  graphics::SharedDeviceChildHandle<graphics::Buffer> upload_buf_;
  graphics::SharedDeviceChildHandle<graphics::Fence> upload_fence_;
  std::byte* upload_ptr_{nullptr};
  UINT64 upload_buf_current_offset_{0};
  std::mutex upload_mutex_;

  std::vector<KeepAliveRecord> resources_to_keep_alive_;
  std::mutex keep_alive_resources_mutex_;
};


constexpr auto RenderManager::GetMaxGpuQueuedFrames() -> UINT {
  return max_gpu_queued_frames_;
}


constexpr auto RenderManager::GetMaxFramesInFlight() -> UINT {
  return max_frames_in_flight_;
}


auto TransformProjectionMatrixForRendering(Matrix4 const& proj_mtx) noexcept -> Matrix4;

auto GenerateSphereMesh(float radius, int latitudes, int longitudes, std::vector<Vector3>& vertices,
                        std::vector<Vector3>& normals, std::vector<Vector2>& uvs,
                        std::vector<std::uint32_t>& indices) -> void;
}
