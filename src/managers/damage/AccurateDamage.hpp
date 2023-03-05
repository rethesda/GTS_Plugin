#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
	class AccurateDamage : public EventListener  {
		public:
			[[nodiscard]] static AccurateDamage& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			virtual void UnderFootEvent(const UnderFoot& evt) override;

			void GrabActor(Actor* giant, Actor* tiny, std::string_view findbone);
			void DoAccurateCollision(Actor* actor);
			void DoSizeDamage(Actor* giant, Actor* tiny, float totaldamage, float mult, bool DoDamage);
	};
}
