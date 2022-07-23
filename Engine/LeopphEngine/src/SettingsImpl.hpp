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
			void operator=(SettingsImpl const& other) = delete;

			SettingsImpl(SettingsImpl&& other) = delete;
			void operator=(SettingsImpl&& other) = delete;

			~SettingsImpl() noexcept override = default;

		private:
			void Serialize() const;
			void Deserialize();

			// If serialization needs to happen, we do it at the end of frame.
			void OnEventReceived(EventReceiver<FrameCompleteEvent>::EventParamType) override;

			// Updates local window settings cache when serializable window configuration changes happen.
			void OnEventReceived(EventReceiver<WindowEvent>::EventParamType event) override;

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
