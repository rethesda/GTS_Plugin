#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/AttachPoint.hpp"
#include "managers/Controllers/HugController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
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


/*Event used in the behaviours to transition between most behaviour states
 */


namespace {

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////E V E N T S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void HugAttemptEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		if (IsGtsBusy(player)) {
			return;
		}
        auto& Hugging = HugAnimationController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(player)/3);
		}
		std::vector<Actor*> preys = Hugging.GetHugTargetsInFront(player, numberOfPrey);
		for (auto prey: preys) {
			Hugging.StartHug(player, prey);
		}
		AnimationManager::StartAnim("Huggies_Try", player);
	}

	void HugShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		AnimationManager::StartAnim("Huggies_Shrink", player);
	}
	void HugReleaseEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto huggedActor = HugShrink::GetHuggiesActor(player);
		if (!huggedActor) {
			return;
		}
		AnimationManager::StartAnim("Huggies_Spare", player);
	}
}

namespace Gts {
	HugShrink& HugShrink::GetSingleton() noexcept {
		static HugShrink instance;
		return instance;
	}

	std::string HugShrink::DebugName() {
		return "HugShrink";
	}

	void HugShrink::DetachActorTask(Actor* giant) {
		std::string name = std::format("Huggies_{}", giant->formID);
		TaskManager::Cancel(name);
	}

	void HugShrink::AttachActorTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("Huggies_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			// Exit on death
			float sizedifference = get_target_scale(giantref)/get_target_scale(tinyref);

			if (tinyref->IsDead() || sizedifference > 6.0) {
				HugShrink::Release(giantref);
				PushActorAway(giantref, tinyref, 0.1);
				DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.30, 1.0);
				return false;
			}

            if (!AttachToObjectA(gianthandle, tinyhandle)) {
                // Unable to attach
                return false;
            }
			
			// All good try another frame
			return true;
		});
	}


	void HugShrink::HugActor_Actor(Actor* giant, TESObjectREFR* tiny, float strength) {
		HugShrink::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void HugShrink::HugActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		HugShrink::HugActor(giant, tiny, 1.0);
	}

	void HugShrink::Reset() {
		this->data.clear();
	}

	void HugShrink::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void HugShrink::Release(Actor* giant) {
		HugShrink::GetSingleton().data.erase(giant);
	}

	TESObjectREFR* HugShrink::GetHuggiesObj(Actor* giant) {
		try {
			auto& me = HugShrink::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* HugShrink::GetHuggiesActor(Actor* giant) {
		auto obj = HugShrink::GetHeldObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void HugShrink::RegisterEvents() {
        InputManager::RegisterInputEvent("HugAttemptEvent", HugAttemptEvent);
		InputManager::RegisterInputEvent("HugReleaseEvent", HugReleaseEvent);
		InputManager::RegisterInputEvent("HugShrinkEvent", HugShrinkEvent);

		AnimationManager::RegisterEvent("GTSBEH_GrabExit", "Hugs", GTSBEH_GrabExit);
		AnimationManager::RegisterEvent("GTSBEH_AbortGrab", "Hugs", GTSBEH_AbortGrab);
	}

	void HugShrink::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Huggies_Try", "Hugs", "GTSBEH_HugAbsorbStart_A");
        AnimationManager::RegisterTrigger("Huggies_Try_Victim", "Hugs", "GTSBEH_HugAbsorbStart_V");
        AnimationManager::RegisterTrigger("Huggies_Shrink", "Hugs", " GTSBEH_HugAbsorbAtk");
        AnimationManager::RegisterTrigger("Huggies_Spare", "Hugs", "GTSBEH_HugAbsorbExitLoop");
        AnimationManager::RegisterTrigger("Huggies_Cancel", "Hugs", " GTSBEH_PairedAbort";)
	}

	HugShrinkData::HugShrinkData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}


//Beh's:
/*
        GTSBEH_GrabStart
        GTSBEH_GrabVore
        GTSBEH_GrabAttack
        GTSBEH_GrabThrow
        GTSBEH_GrabRelease

        GTSBeh_GrabExit
        GTSBEH_AbortGrab (Similar to GTSBEH_Exit but Grab only)
 */