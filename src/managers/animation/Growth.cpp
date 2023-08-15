#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Growth.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

    void SetHalfLife(Actor* actor, float value) {
        auto& Persist = Persistent::GetSingleton();
        auto actor_data = Persist.GetData(actor);
        if (actor_data) {
			actor_data->half_life = value; 
		}
    }

    void CancelGrowth(Actor* actor) {
        std::string name = std::format("ManualGrowth_{}", actor->formID);
        TaskManager::Cancel(name);
        SetHalfLife(actor, 1.0);
    }

    void GrowthTask(Actor* actor) {
        if (!actor) {
            return;
        }
        SetHalfLife(actor, 0.02);
		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = actor->CreateRefHandle();
		std::string name = std::format("ManualGrowth_{}", actor->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto caster = gianthandle.get().get();
            float timepassed = Time::WorldTimeElapsed() - Start;
			float elapsed = std::clamp(timepassed * AnimationManager::GetAnimSpeed(caster), 0.01f, 1.2f);
			float multiply = bezier_curve(elapsed, 0, 0.9, 1, 1, 2.0);
            log::info("Elapsed {}, Multiply: {}", elapsed, multiply);
			
			float caster_scale = get_visual_scale(caster);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale() * multiply);
			Grow(caster, 0.0060 * stamina * multiply, 0.0);

			Rumble::Once("GrowButton", caster, 3.0 * stamina, 0.05);
			return true;
		});
	}

	void GTSGrowth_Enter(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStart(AnimationEventData& data) {
        GrowthTask(&data.giant);
    }
    void GTSGrowth_SpurtSlowdownPoint(AnimationEventData& data) {
    }
    void GTSGrowth_SpurtStop(AnimationEventData& data) {
        CancelGrowth(&data.giant);
    }
    void GTSGrowth_Exit(AnimationEventData& data) {
    }
}

namespace Gts
{
	void AnimationGrowth::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSGrowth_Enter", "Growth", GTSGrowth_Enter);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtStart", "Growth", GTSGrowth_SpurtStart);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtSlowdownPoint", "Growth", GTSGrowth_SpurtSlowdownPoint);
        AnimationManager::RegisterEvent("GTSGrowth_SpurtStop", "Growth", GTSGrowth_SpurtStop);
        AnimationManager::RegisterEvent("GTSGrowth_Exit", "Growth", GTSGrowth_Exit);
	}

	void AnimationGrowth::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TriggerGrowth", "Growth", "GTSBeh_Grow_Trigger");
	}
}