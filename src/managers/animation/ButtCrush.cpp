#include "managers/animation/Controllers/ButtCrushController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    bool CanDoButtCrush(Actor* actor) {
		static Timer Default = Timer(30);
		static Timer UnstableGrowth = Timer(25.5);
        static Timer LoomingDoom = Timer(20.4);
		bool lvl70 = Runtime::HasPerk(actor, "ButtCrush_UnstableGrowth");
        bool lvl100 = Runtime::HasPerk(actor, "ButtCrush_LoomingDoom");
        if (lvl100) {
            return LoomingDoom.ShouldRunFrame();
        } else if (lvl70) {
			return UnstableGrowth.ShouldRunFrame();
		} else {
			return Default.ShouldRunFrame();
		}
	}

    void AttachToObjectBTask(Actor* giant, Actor* tiny) {
        SetBeingEaten(tiny, true);
        std::string name = std::format("ButtCrush_{}", tiny->formID);
        auto tinyhandle = tiny->CreateRefHandle();
        auto gianthandle = giant->CreateRefHandle();
        TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			if (!AttachToObjectB(giantref, tinyref)) {
                SetBeingEaten(tiny, false);
				return false;
			} if (!IsButtCrushing(giantref)) {
                SetBeingEaten(tiny, false);
				return false;
			} if (tinyref->IsDead()) {
                SetBeingEaten(tiny, false);
				return false;
			}
			return true;
		});
    }

    void TrackButt(Actor* giant, bool enable) {
        if (AllowFeetTracking()) {
            auto& sizemanager = SizeManager::GetSingleton();
            sizemanager.SetActionBool(giant, enable, 8.0);
        }
    }

    void ModGrowthCount(Actor* giant, float value, bool reset) {
        auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			transient->ButtCrushGrowthAmount += value;
            if (reset) {
                transient->ButtCrushGrowthAmount = 0.0;
            }
		}
    }

    void SetBonusSize(Actor* giant, float value, bool reset) {
        auto saved_data = Persistent::GetSingleton().GetData(giant);
        if (saved_data) {
            saved_data->bonus_max_size += value;
            if (reset) {
                mod_target_scale(giant, -saved_data->bonus_max_size);
                saved_data->bonus_max_size = 0;
            }
        } 
    }

    float GetGrowthCount(Actor* giant) {
        auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			return transient->ButtCrushGrowthAmount;
		}
        return 1.0;
    }

    float GetGrowthLimit(Actor* actor) {
        float limit = 0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_GrowingDisaster")) {
            limit += 2.0;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
            limit += 2.0;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
            limit += 4.0;
        }
        return limit;
    }

    float GetButtCrushCost(Actor* actor) {
        float cost = 1.0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
            cost -= 0.20;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
            cost -= 0.25;
        }
        return cost;
    }

    float GetButtCrushDamage(Actor* actor) {
        float damage = 1.0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
            damage += 0.30;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
            damage += 0.70;
        }
        return damage;
    }

    void GTSButtCrush_MoveBody_MixFrameToLoop(AnimationEventData& data) {
        auto giant = &data.giant;
        TrackButt(giant, true);
    }

    void GTSButtCrush_GrowthStart(AnimationEventData& data) {
        auto giant = &data.giant;
        ModGrowthCount(giant, 1.0, false);
        float scale = get_visual_scale(giant);
        float bonus = 0.24 * GetGrowthCount(giant) * (1.0 + (scale/10));
        SpringGrow_Free(giant, bonus, 0.3, "ButtCrushGrowth");
        SetBonusSize(giant, bonus, false);
        Runtime::PlaySoundAtNode("growthSound", giant, 1.0, 1.0, "NPC Pelvis [Pelv]");
		Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
    }

    void GTSButtCrush_FootstepR(AnimationEventData& data) { 
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.45;
		}
        Rumble::Once("FS_R", &data.giant, 2.20, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.4, 1.45 , 10, 0.25, FootEvent::Right, 1.0);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25 * data.animSpeed, 1.4, FootEvent::Right, 0.95);
    }

    void GTSButtCrush_FootstepL(AnimationEventData& data) { 
        float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.45;
		}
        Rumble::Once("FS_L", &data.giant, 2.20, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.4, 1.45 , 10, 0.25, FootEvent::Left, 1.0);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * launch * perk, 2.25 * data.animSpeed, 1.4, FootEvent::Left, 0.95);
    }

    void GTSButtCrush_HandImpactR(AnimationEventData& data) {
        auto giant = &data.giant;
        float scale = get_visual_scale(giant);
        DoCrawlingFunctions(giant, scale, 1.0, CrawlEvent::RightHand, "RightHand", 18, 14);
    }

    void GTSButtCrush_FallDownImpact(AnimationEventData& data) {
        auto giant = &data.giant;

        float perk = GetPerkBonus_Basics(&data.giant);
        float launch = 1.0;
        float dust = 1.0;
        
        if (HasSMT(giant)) {
            launch = 1.25;
            dust = 1.25;
        }

        SetBonusSize(giant, 0.0, true);

        float damage = GetButtCrushDamage(giant);
        auto ThighL = find_node(giant, "NPC L Thigh [LThg]");
		auto ThighR = find_node(giant, "NPC R Thigh [RThg]");
        auto ButtR = find_node(giant, "NPC R Butt");
        auto ButtL = find_node(giant, "NPC L Butt");
        if (ButtR && ButtL) {
            if (ThighL && ThighR) {
                DoDamageAtPoint(giant, 26, 800.0 * damage, ThighL, 4, 0.70, 0.85);
                DoDamageAtPoint(giant, 26, 800.0 * damage, ThighR, 4, 0.70, 0.85);
                DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Right, "NPC R Butt");
                DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Left, "NPC L Butt");
                DoFootstepSound(giant, 1.25, FootEvent::Right, RNode);
                DoLaunch(&data.giant, 28.00 * launch * perk, 4.20, 1.4, FootEvent::Butt, 1.20);
                Rumble::Once("Butt_L", &data.giant, 3.60 * damage, 0.02, "NPC R Butt");
                Rumble::Once("Butt_R", &data.giant, 3.60 * damage, 0.02, "NPC L Butt");
            }
        }
        ModGrowthCount(giant, 0, true); // Reset limit
        TrackButt(giant, false);
    }

    void GTSButtCrush_Exit(AnimationEventData& data) {
        auto giant = &data.giant;
        ModGrowthCount(giant, 0, true); // Reset limit
        TrackButt(giant, false);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    ///                     T R I G G E R S
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////


    void ButtCrushStartEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
        if (CanDoButtCrush(player)) {
            float WasteStamina = 100.0 * GetButtCrushCost(player);
            DamageAV(player, ActorValue::kStamina, WasteStamina);
            if (Runtime::HasPerk(player, "ButtCrush_NoEscape")) {
                auto& ButtCrush = ButtCrushController::GetSingleton();
                std::size_t numberOfPrey = 1;
                if (Runtime::HasPerk(player, "MassVorePerk")) {
                    numberOfPrey = 3 + (get_visual_scale(player)/3);
                }
                std::vector<Actor*> preys = ButtCrush.GetButtCrushTargets(player, numberOfPrey);
                for (auto prey: preys) {
                    ButtCrush.StartButtCrush(player, prey);
                    AttachToObjectBTask(player, prey);
                }
            } else {
                AnimationManager::StartAnim("ButtCrush_StartFast", player);
            }
        } else {
			TiredSound(player, "Butt Crush is on a cooldown");
		}
	}

    void ButtCrushGrowEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
        if (IsButtCrushing(player)) {
            float GrowthCount = GetGrowthLimit(player);
            bool CanGrow = ButtCrush_IsAbleToGrow(player, GrowthCount);
            if (CanGrow) {
                float WasteStamina = 35.0 * GetButtCrushCost(player);
                DamageAV(player, ActorValue::kStamina, WasteStamina);
                AnimationManager::StartAnim("ButtCrush_Growth", player);
            } else {
                TiredSound(player, "Your body can't grow any further");
            }
        }
	}

    void ButtCrushAttackEvent(const InputEventData& data) {
        auto player = PlayerCharacter::GetSingleton();
        if (IsButtCrushing(player)) {
            AnimationManager::StartAnim("ButtCrush_Attack", player);
        }
    }
}

namespace Gts
{
	void AnimationButtCrush::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSButtCrush_Exit", "ButtCrush", GTSButtCrush_Exit);
        AnimationManager::RegisterEvent("GTSButtCrush_GrowthStart", "ButtCrush", GTSButtCrush_GrowthStart);
        AnimationManager::RegisterEvent("GTSButtCrush_FallDownImpact", "ButtCrush", GTSButtCrush_FallDownImpact);
        AnimationManager::RegisterEvent("GTSButtCrush_HandImpactR", "ButtCrush", GTSButtCrush_HandImpactR);
        AnimationManager::RegisterEvent("GTSButtCrush_FootstepR", "ButtCrush", GTSButtCrush_FootstepR);
        AnimationManager::RegisterEvent("GTSButtCrush_FootstepL", "ButtCrush", GTSButtCrush_FootstepL);
        AnimationManager::RegisterEvent("GTSButtCrush_MoveBody_MixFrameToLoop", "ButtCrush", GTSButtCrush_MoveBody_MixFrameToLoop);
        
        InputManager::RegisterInputEvent("ButtCrushStart", ButtCrushStartEvent);
        InputManager::RegisterInputEvent("ButtCrushGrow", ButtCrushGrowEvent);
		InputManager::RegisterInputEvent("ButtCrushAttack", ButtCrushAttackEvent);
	}

    void AnimationButtCrush::RegisterTriggers() {
		AnimationManager::RegisterTrigger("ButtCrush_Start", "ButtCrush", "GTSBEH_ButtCrush_Start");
        AnimationManager::RegisterTrigger("ButtCrush_Attack", "ButtCrush", "GTSBEH_ButtCrush_Attack");
        AnimationManager::RegisterTrigger("ButtCrush_Growth", "ButtCrush", "GTSBEH_ButtCrush_Grow");
        AnimationManager::RegisterTrigger("ButtCrush_StartFast", "ButtCrush", "GTSBEH_ButtCrush_StartFast");
	}
}