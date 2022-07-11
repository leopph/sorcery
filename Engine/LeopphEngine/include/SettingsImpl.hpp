#pragma once

#include "EventReceiver.hpp"
#include "FrameCompleteEvent.hpp"
#include "Settings.hpp"
#include "WindowEvent.hpp"


namespace leopph::internal
{
	class SettingsImpl final : public Settings, public EventReceiver<FrameCompleteEvent>, public EventReceiver<WindowEvent>
	{
		public:
			SettingsImpl();

			SettingsImpl(SettingsImpl const& other) = delete;
			auto operator=(SettingsImpl const& other) -> void = delete;

			SettingsImpl(SettingsImpl&& other) = delete;
			auto operator=(SettingsImpl&& other) -> void = delete;

			~SettingsImpl() noexcept override = default;

		private:
			auto Serialize() const -> void;
			auto Deserialize() -> void;

			// If serialization needs to happen, we do it at the end of frame.
			auto OnEventReceived(EventReceiver<FrameCompleteEvent>::EventParamType) -> void override;

			// Updates local window settings cache when serializable window configuration changes happen.
			auto OnEventReceived(EventReceiver<WindowEvent>::EventParamType event) -> void override;

			static char const* const JSON_SHADER_LOC;
			static char const* const JSON_DIR_SHADOW_RES;
			static char const* const JSON_SPOT_SHADOW_RES;
			static char const* const JSON_POINT_SHADOW_RES;
			static char const* const JSON_MAX_SPOT;
			static char const* const JSON_MAX_POINT;
			static char const* const JSON_API;
			static char const* const JSON_PIPE;
			static char const* const JSON_DIR_CORRECT;
			static char const* const JSON_VSYNC;
			static char const* const JSON_RES_W;
			static char const* const JSON_RES_H;
			static char const* const JSON_REND_MULT;
			static char const* const JSON_FULLSCREEN;
			static char const* const JSON_GAMMA;
	};
}
