#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

namespace {
	bool Escaped(Actor* giant, Actor* tiny, float strength) {
		float tiny_chance = ((rand() % 100000) / 100000.0f) * get_visual_scale(tiny) ;
		float giant_chance = ((rand() % 100000) / 100000.0f) * strength * get_visual_scale(giant);
		return (tiny_chance > giant_chance);
	}
}

namespace Gts {
	Grab& Grab::GetSingleton() noexcept {
		static Grab instance;
		return instance;
	}

	std::string Grab::DebugName() {
		return "Grab";
	}

	void Grab::Update() {
		for (auto &[giant, data]: this->data) {
			if (!giant) {
				continue;
			}
			auto tiny = data.tiny;
			if (!tiny) {
				continue;
			}

			auto bone = find_node(giant, "NPC L Finger02 [LF02]");
			if (!bone) {
				return;
			}

			float giantScale = get_visual_scale(giant);

			NiPoint3 giantLocation = giant->GetPosition();
			NiPoint3 tinyLocation = tiny->GetPosition();

			tiny->SetPosition(bone->world.translate);
      Actor* tiny_is_actor = TESForm::LookupByID<Actor>(tiny->formID);
      if (tiny_is_actor) {
  			auto charcont = tiny_is_actor->GetCharController();
  			if (charcont) {
  				charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
  			}
      } else {
        // TODO: Work out method for generic objects
      }

			// TODO: Add escape
			// if Escaped(giant, tiny, data.strength) {
			//   this->data.erase(giant);
			// }
		}
	}


	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny, float strength) {
		Grab::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		Grab::GrabActor(giant, tiny, 1.0);
	}

	void Grab::Release(Actor* giant) {
		Grab::GetSingleton().data.erase(giant);
	}

  Actor* Grab::GetHeldActor(Actor* giant) {
    try {
      auto& me = Grab::GetSingleton();
      return me.data.at(giant).tiny;
    } catch (std::out_of_range e) {
      return nullptr;
    }
  }

	GrabData::GrabData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}

}
