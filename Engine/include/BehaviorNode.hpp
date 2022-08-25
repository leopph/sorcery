#pragma once

#include "Node.hpp"
#include "Types.hpp"

#include <vector>


namespace leopph
{
	class BehaviorNode : public Node
	{
		public:
			inline virtual void on_frame_update();

			[[nodiscard]] LEOPPHAPI i32 get_update_index() const;
			LEOPPHAPI void set_update_index(i32 index);

		protected:
			LEOPPHAPI BehaviorNode();
			LEOPPHAPI BehaviorNode(BehaviorNode const& other);
			LEOPPHAPI BehaviorNode(BehaviorNode&& other) noexcept;

		public:
			LEOPPHAPI ~BehaviorNode() override;

		protected:
			BehaviorNode& operator=(BehaviorNode&& other) = default;
			BehaviorNode& operator=(BehaviorNode const& other) noexcept = default;

		public:
			static void update_all();

		private:
			i32 mUpdateIndex{0};

			static std::vector<BehaviorNode*> sInstances;
	};



	inline void BehaviorNode::on_frame_update()
	{}
}
