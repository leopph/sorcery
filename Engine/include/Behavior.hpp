#pragma once

#include "Component.hpp"
#include "Types.hpp"

#include <vector>


namespace leopph
{
	class Behavior : public Component
	{
		public:
			inline virtual void on_frame_update();

			[[nodiscard]] LEOPPHAPI i32 get_update_index() const;
			LEOPPHAPI void set_update_index(i32 index);

		protected:
			LEOPPHAPI Behavior();

		public:
			Behavior(Behavior const& other) = delete;
			Behavior(Behavior&& other) = delete;

			Behavior& operator=(Behavior&& other) = delete;
			Behavior& operator=(Behavior const& other) = delete;

			LEOPPHAPI ~Behavior() override;


			static void update_all();

		private:
			i32 mUpdateIndex{0};

			static std::vector<Behavior*> sInstances;
	};



	inline void Behavior::on_frame_update()
	{}
}
