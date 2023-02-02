#pragma once
// Module that holds data that is not persistent across saves
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct TempActorData {
		float base_height;
		float base_volume;
		bool wearingHh;
		float base_walkspeedmult;
		float char_weight;
		float shoe_weight;
		float health_storage = 1.0;
		float speedmult_storage = 1.0;
		float damage_storage = 1.0;
		bool is_teammate;
	};

	class Transient : public EventListener {
		public:
			[[nodiscard]] static Transient& GetSingleton() noexcept;

			TempActorData* GetData(TESObjectREFR* object);
			TempActorData* GetActorData(Actor* actor);

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;
		private:

			mutable std::mutex _lock;
			std::unordered_map<FormID, TempActorData> _actor_data;
	};
}
