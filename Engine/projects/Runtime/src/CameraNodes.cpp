#include "CameraNodes.hpp"

#include "Context.hpp"
#include "Logger.hpp"
#include "Math.hpp"
#include "Node.hpp"
#include "Renderer.hpp"
#include "Window.hpp"

#include <algorithm>
#include <utility>


namespace leopph
{
	f32 CameraNode::get_near_clip_plane() const
	{
		return mNear;
	}



	void CameraNode::set_near_clip_plane(f32 const near)
	{
		mNear = near;
	}



	f32 CameraNode::get_far_clip_plane() const
	{
		return mFar;
	}



	void CameraNode::set_far_clip_plane(f32 const far)
	{
		mFar = far;
	}



	std::variant<Color, std::shared_ptr<Skybox>> const& CameraNode::get_background() const
	{
		return mBackground;
	}



	void CameraNode::set_background(std::variant<Color, std::shared_ptr<Skybox>> background)
	{
		mBackground = std::move(background);
	}



	Extent<f32> const& CameraNode::get_viewport() const
	{
		return mViewport;
	}



	void CameraNode::set_viewport(Extent<f32> const& viewport)
	{
		mViewport = viewport;

		auto const& window = get_main_window();
		auto const windowWidth = static_cast<f32>(window.get_width());
		auto const windowHeight = static_cast<f32>(window.get_height());

		mWindowExtent =
		{
			.offsetX = static_cast<u32>(mViewport.offsetX * windowWidth),
			.offsetY = static_cast<u32>(mViewport.offsetY * windowHeight),
			.width = static_cast<u32>(mViewport.width * windowWidth),
			.height = static_cast<u32>(mViewport.height * windowHeight)
		};

		mAspectRatio = static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height);
	}



	Extent<u32> CameraNode::get_window_extents() const
	{
		return mWindowExtent;
	}



	void CameraNode::set_window_extents(Extent<u32> const& extent)
	{
		auto const& window = get_main_window();

		mWindowExtent.offsetX = std::clamp<u32>(extent.offsetX, 0, window.get_width());
		mWindowExtent.offsetY = std::clamp<u32>(extent.offsetY, 0, window.get_height());
		mWindowExtent.width = mWindowExtent.offsetX + extent.width > window.get_width() ? 0 : extent.width;
		mWindowExtent.height = mWindowExtent.offsetY + extent.height > window.get_height() ? 0 : extent.height;

		auto const extentWidth = static_cast<f32>(mWindowExtent.width);
		auto const extentHeight = static_cast<f32>(mWindowExtent.height);

		mAspectRatio = extentWidth / extentHeight;

		auto const windowWidth = static_cast<f32>(window.get_width());
		auto const windowHeight = static_cast<f32>(window.get_height());

		mViewport.offsetX = windowWidth / static_cast<f32>(mWindowExtent.offsetX);
		mViewport.offsetY = windowHeight / static_cast<f32>(mWindowExtent.offsetY);
		mViewport.width = windowWidth / extentWidth;
		mViewport.height = windowHeight / extentHeight;
	}



	f32 CameraNode::get_aspect_ratio() const
	{
		return mAspectRatio;
	}



	void CameraNode::enable()
	{
		if (!mEnabled)
		{
			internal::get_renderer().register_camera(this);
			mEnabled = true;
		}
	}



	void CameraNode::disable()
	{
		if (mEnabled)
		{
			internal::get_renderer().unregister_camera(this);
			mEnabled = false;
		}
	}



	bool CameraNode::is_enabled() const
	{
		return mEnabled;
	}



	Matrix4 CameraNode::build_view_matrix() const
	{
		// inv(T) * inv(R)
		return Matrix4::translate(-get_position()) * Matrix4{get_rotation()}.transpose();
	}



	Vector2 CameraNode::transform_to_viewport(Vector3 const& vector) const
	{
		Vector4 homoPos{vector, 1};
		homoPos *= build_view_matrix() * build_projection_matrix();
		Vector2 ret{homoPos};
		ret /= homoPos[3];
		ret *= 0.5f;
		ret += Vector2{0.5f};
		return ret;
	}



	CameraNode::CameraNode() :
		mWindowExtent
		{
			[this]
			{
				auto const& window = get_main_window();
				auto const windowWidth = static_cast<f32>(window.get_width());
				auto const windowHeight = static_cast<f32>(window.get_height());

				return Extent<u32>
				{
					.offsetX = static_cast<u32>(mViewport.offsetX * windowWidth),
					.offsetY = static_cast<u32>(mViewport.offsetY * windowHeight),
					.width = static_cast<u32>(mViewport.width * windowWidth),
					.height = static_cast<u32>(mViewport.height * windowHeight)
				};
			}()
		},
		mAspectRatio{static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height)}
	{
		internal::get_renderer().register_camera(this);
	}



	CameraNode::CameraNode(CameraNode const& other) :
		Node{other},
		mBackground{other.mBackground},
		mNear{other.mNear},
		mFar{other.mFar},
		mViewport{other.mViewport},
		mWindowExtent{other.mWindowExtent},
		mAspectRatio{other.mAspectRatio},
		mEnabled{other.mEnabled}
	{
		if (mEnabled)
		{
			internal::get_renderer().register_camera(this);
		}
	}



	CameraNode::CameraNode(CameraNode&& other) noexcept :
		Node{std::move(other)},
		mBackground{other.mBackground},
		mNear{other.mNear},
		mFar{other.mFar},
		mViewport{other.mViewport},
		mWindowExtent{other.mWindowExtent},
		mAspectRatio{other.mAspectRatio},
		mEnabled{other.mEnabled}
	{
		if (mEnabled)
		{
			internal::get_renderer().register_camera(this);
		}
	}



	CameraNode::~CameraNode()
	{
		internal::get_renderer().unregister_camera(this);
	}



	CameraNode& CameraNode::operator=(CameraNode const& other)
	{
		if (this == &other)
		{
			return *this;
		}

		if (mEnabled)
		{
			internal::get_renderer().unregister_camera(this);
		}

		mBackground = other.mBackground;
		mNear = other.mNear;
		mFar = other.mFar;
		mViewport = other.mViewport;
		mWindowExtent = other.mWindowExtent;
		mAspectRatio = other.mAspectRatio;
		mEnabled = other.mEnabled;

		if (mEnabled)
		{
			internal::get_renderer().register_camera(this);
		}

		return *this;
	}



	CameraNode& CameraNode::operator=(CameraNode&& other) noexcept
	{
		return *this = other;
	}



	void CameraNode::on_event(WindowEvent const& event)
	{
		mWindowExtent =
		{
			.offsetX = static_cast<u32>(mViewport.offsetX * static_cast<f32>(event.width)),
			.offsetY = static_cast<u32>(mViewport.offsetY * static_cast<f32>(event.height)),
			.width = static_cast<u32>(mViewport.width * static_cast<f32>(event.width)),
			.height = static_cast<u32>(mViewport.height * static_cast<f32>(event.height))
		};

		mAspectRatio = static_cast<f32>(mWindowExtent.width) / static_cast<f32>(mWindowExtent.height);
	}



	f32 OrthographicCameraNode::get_size(Side const side) const
	{
		if (side == Side::Horizontal)
		{
			return mHorizSize;
		}

		if (side == Side::Vertical)
		{
			return mHorizSize / get_aspect_ratio();
		}

		Logger::get_instance().warning(std::format("Invalid side [{}] while returning camera size. Returning 0.", static_cast<int>(side)));
		return 0;
	}



	void OrthographicCameraNode::size(f32 const size, Side const side)
	{
		if (side == Side::Horizontal)
		{
			mHorizSize = size;
		}
		else if (side == Side::Vertical)
		{
			mHorizSize = size * get_aspect_ratio();
		}
		else
		{
			Logger::get_instance().warning(std::format("Invalid side [{}] while setting camera size. Ignoring.", static_cast<int>(side)));
		}
	}



	Matrix4 OrthographicCameraNode::build_projection_matrix() const
	{
		auto static constexpr half = 1.f / 2.f;
		auto const x = mHorizSize * half;
		auto const y = mHorizSize / get_aspect_ratio() * half;
		return Matrix4::orthographic(-x, x, y, -y, get_near_clip_plane(), get_far_clip_plane());
	}



	Frustum OrthographicCameraNode::build_frustum() const
	{
		auto static constexpr half = 1.f / 2.f;

		auto const x = mHorizSize * half;
		auto const y = mHorizSize / get_aspect_ratio() * half;

		auto const near = get_near_clip_plane();
		auto const far = get_far_clip_plane();

		return Frustum
		{
			.rightTopNear{x, y, near},
			.leftTopNear{-x, y, near},
			.leftBottomNear{-x, -y, near},
			.rightBottomNear{x, -y, near},
			.rightTopFar = {x, y, far},
			.leftTopFar{-x, y, far},
			.leftBottomFar{-x, -y, far},
			.rightBottomFar = {x, -y, far},
		};
	}



	f32 PerspectiveCameraNode::get_fov(Side const side) const
	{
		if (side == Side::Horizontal)
		{
			return mHorizFovDeg;
		}

		if (side == Side::Vertical)
		{
			return convert_fov(mHorizFovDeg, false);
		}

		Logger::get_instance().warning(std::format("Invalid side [{}] while querying camera field of view. Returning 0.", static_cast<int>(side)));
		return 0;
	}



	void PerspectiveCameraNode::set_fov(f32 const degrees, Side const side)
	{
		if (side == Side::Horizontal)
		{
			mHorizFovDeg = degrees;
		}
		else if (side == Side::Vertical)
		{
			mHorizFovDeg = convert_fov(degrees, true);
		}
		else
		{
			Logger::get_instance().warning(std::format("Invalid side [{}] while setting camera field of view. Ignoring.", static_cast<int>(side)));
		}
	}



	Matrix4 PerspectiveCameraNode::build_projection_matrix() const
	{
		auto const fov{to_radians(convert_fov(mHorizFovDeg, false))};
		return Matrix4::perspective(fov, get_aspect_ratio(), get_near_clip_plane(), get_far_clip_plane());
	}



	Frustum PerspectiveCameraNode::build_frustum() const
	{
		auto const tanHalfHorizFov{std::tan(to_radians(get_fov(Side::Horizontal)) / 2.0f)};
		auto const tanHalfVertFov{std::tan(to_radians(get_fov(Side::Vertical)) / 2.0f)};

		auto const near = get_near_clip_plane();
		auto const far = get_far_clip_plane();

		auto const xn = near * tanHalfHorizFov;
		auto const xf = far * tanHalfHorizFov;
		auto const yn = near * tanHalfVertFov;
		auto const yf = far * tanHalfVertFov;

		return Frustum
		{
			.rightTopNear{xn, yn, near},
			.leftTopNear{-xn, yn, near},
			.leftBottomNear{-xn, -yn, near},
			.rightBottomNear{xn, -yn, near},
			.rightTopFar{xf, yf, far},
			.leftTopFar{-xf, yf, far},
			.leftBottomFar{-xf, -yf, far},
			.rightBottomFar{xf, -yf, far},
		};
	}



	f32 PerspectiveCameraNode::convert_fov(f32 const fov, bool const vert2Horiz) const
	{
		if (vert2Horiz)
		{
			return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) * get_aspect_ratio()));
		}

		return to_degrees(2.0f * std::atan(std::tan(to_radians(fov) / 2.0f) / get_aspect_ratio()));
	}
}
