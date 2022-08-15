#pragma once

#include "Component.hpp"
#include "Types.hpp"

#include <vector>


namespace leopph
{
	class Behavior : public Component
	{
		public:
			virtual void on_frame_update() = 0;

			[[nodiscard]] LEOPPHAPI i32 get_update_index() const;
			LEOPPHAPI void set_update_index(i32 index);

		protected:
			Behavior();

		public:
			Behavior(Behavior const& other) = delete;
			Behavior(Behavior&& other) = delete;

			Behavior& operator=(Behavior&& other) = delete;
			Behavior& operator=(Behavior const& other) = delete;

			LEOPPHAPI ~Behavior() override;


			static void update_all_behaviors();

		private:
			i32 mUpdateIndex{0};

			static std::vector<Behavior*> sAllBehaviors;
	};
}
