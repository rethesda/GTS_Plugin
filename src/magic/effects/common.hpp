#pragma once
#include "managers/GtsSizeManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
// Module that handles various magic effects

namespace {
	const float MASTER_POWER = 2.0;
}

namespace Gts {
	inline float TimeScale() {
		const float BASE_FPS = 60.0; // Parameters were optimised on this fps
		return Time::WorldTimeDelta() * BASE_FPS;
	}

	inline void AdjustSizeLimit(float value)  // A function that adjusts Size Limit (Globals)
	{
		float progressionMultiplier = Runtime::GetFloatOr("ProgressionMultiplier", 1.0);

		auto globalMaxSizeCalc = Runtime::GetFloat("GlobalMaxSizeCalc");
		if (globalMaxSizeCalc < 10.0) {
			Runtime::SetFloat("GlobalMaxSizeCalc", globalMaxSizeCalc + (value * 50 * progressionMultiplier * TimeScale())); // Always apply it
		}

		auto selectedFormula = Runtime::GetInt("SelectedSizeFormula");
		if (selectedFormula) {
			if (selectedFormula >= 2.0) {
				auto globalMassSize = Runtime::GetFloat("MassBasedSizeLimit"); // <- Applies it
				auto sizeLimit = Runtime::GetFloat("sizeLimit");
				if (globalMassSize < sizeLimit) {
					Runtime::SetFloat("MassBasedSizeLimit", globalMassSize + value * progressionMultiplier * TimeScale());
				}
			}
		}
	}

	inline float CalcEffeciency(Actor* caster, Actor* target) {
		const float DRAGON_PEANLTY = 0.14;
		float casterlevel = clamp(1.0, 500.0, caster->GetLevel());
		float targetlevel = clamp(1.0, 500.0, target->GetLevel());
		float progression_multiplier = Runtime::GetFloatOr("ProgressionMultiplier", 1.0);
		float GigantismCaster = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
		float SizeHunger = 1.0 + SizeManager::GetSingleton().GetSizeHungerBonus(caster)/100;
		float GigantismTarget = clamp(0.05, 1.0, 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(target)/100);  // May go negative needs fixing with a smooth clamp
		float efficiency = clamp(0.25, 1.25, (casterlevel/targetlevel)) * progression_multiplier;
		if (IsDragon(target)) {
			efficiency *= DRAGON_PEANLTY;
		}
		if (Runtime::HasMagicEffect(target, "ResistShrinkPotion")) {
			efficiency *= 0.25;
		}

		efficiency *= GigantismCaster * GigantismTarget * SizeHunger;

		return efficiency;
	}

	inline float CalcEffeciency_NoProgression(Actor* caster, Actor* target) {
		const float DRAGON_PEANLTY = 0.14;
		float casterlevel = clamp(1.0, 500.0, caster->GetLevel());
		float targetlevel = clamp(1.0, 500.0, target->GetLevel());
		float progression_multiplier = Runtime::GetFloatOr("ProgressionMultiplier", 1.0);
		float GigantismCaster = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;
		float SizeHunger = 1.0 + SizeManager::GetSingleton().GetSizeHungerBonus(caster)/100;
		float GigantismTarget = clamp(0.05, 1.0, 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(target)/100);  // May go negative needs fixing with a smooth clamp
		float efficiency = clamp(0.25, 1.25, (casterlevel/targetlevel));
		//log::info("LevelDifference: {}, caster level: {}, target level: {}", efficiency, casterlevel, targetlevel);
		if (IsDragon(target)) {
			efficiency *= DRAGON_PEANLTY;
		}
		if (Runtime::HasMagicEffect(target, "ResistShrinkPotion")) {
			efficiency *= 0.25;
		}

		efficiency *= GigantismCaster * GigantismTarget * SizeHunger;
		//log::info("Total Efficiency: {}", efficiency);

		return efficiency;
	}

	inline float CalcPower(Actor* actor, float scale_factor, float bonus) {
		float progression_multiplier = Runtime::GetFloatOr("ProgressionMultiplier", 1.0);
		// y = mx +c
		// power = scale_factor * scale + bonus
		return (get_visual_scale(actor) * scale_factor + bonus) * progression_multiplier * MASTER_POWER * TimeScale();
	}

	inline float CalcPower_NoMult(Actor* actor, float scale_factor, float bonus) {
		// y = mx +c
		// power = scale_factor * scale + bonus
		return (get_visual_scale(actor) * scale_factor + bonus) * MASTER_POWER * TimeScale();
	}

	inline void Grow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		mod_target_scale(actor, CalcPower(actor, scale_factor, bonus));
	}

	inline void CrushGrow(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float modifier = SizeManager::GetSingleton().BalancedMode();
		scale_factor /= modifier;
		bonus /= modifier;
		mod_target_scale(actor, CalcPower(actor, scale_factor, bonus));
	}

	inline void ShrinkActor(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		mod_target_scale(actor, -CalcPower(actor, scale_factor, bonus));
	}

	inline bool Revert(Actor* actor, float scale_factor, float bonus) {
		// amount = scale * a + b
		float amount = CalcPower(actor, scale_factor, bonus);
		float target_scale = get_target_scale(actor);
		float natural_scale = SizeManager::GetSingleton().GetRaceScale(actor);  //get_natural_scale(actor); It behaved weirdly: used to revert my character to x0.87 instead of x1.0.

		if (fabs(target_scale - natural_scale) < amount) {
			set_target_scale(actor, natural_scale);
			return false;
		} else if (target_scale < natural_scale) { // NOLINT
			mod_target_scale(actor, amount);
		} else {
			mod_target_scale(actor, -amount);
		}
		return true;
	}

	inline void Steal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency) {
		effeciency = clamp(0.01, 1.0, effeciency);
		float effeciency_noscale = clamp(0.01, 1.0, CalcEffeciency_NoProgression(to, from));
		//log::info("Efficiency is: {}", effeciency_noscale);
		float amount = CalcPower(from, scale_factor, bonus);
		float amountnomult = CalcPower_NoMult(from, scale_factor, bonus);
		float target_scale = get_visual_scale(from);
		AdjustSizeLimit(0.0001 * scale_factor * target_scale);
		mod_target_scale(from, -amountnomult * 0.55 * effeciency_noscale);
		mod_target_scale(to, amount*effeciency);
	}

	inline void AbsorbSteal(Actor* from, Actor* to, float scale_factor, float bonus, float effeciency) {
		effeciency = clamp(0.0, 1.0, effeciency);
		float amount = CalcPower(from, scale_factor, bonus);
		float target_scale = get_visual_scale(from);
		AdjustSizeLimit(0.0016 * scale_factor * target_scale);
		mod_target_scale(from, -amount);
		mod_target_scale(to, amount*effeciency/10); // < 10 times weaker size steal towards caster. Absorb exclusive.
	}

	inline void Transfer(Actor* from, Actor* to, float scale_factor, float bonus) {
		Steal(from, to, scale_factor, bonus, 1.0); // 100% efficent for friendly steal
	}

	inline void Grow_Ally(Actor* from, Actor* to, float receiver, float caster) {
		float receive = CalcPower(from, receiver, 0);
		float lose = CalcPower(from, receiver, 0);
		float CasterScale = get_target_scale(from);
		if (CasterScale > 1.0) { // We don't want to scale the caster below this limit!
			mod_target_scale(from, -lose);
		}
		mod_target_scale(to, receive);
	}

	inline void TransferSize(Actor* caster, Actor* target, bool dual_casting, float power, float transfer_effeciency, bool smt) {
		const float BASE_POWER = 0.0005;
		const float DUAL_CAST_BONUS = 2.0;
		const float SMT_BONUS = 2.0;
		const float PERK1_BONUS = 1.33;
		const float PERK2_BONUS = 2.0;

		if (Runtime::GetBool("ProtectEssentials") && target->IsEssential()) {
			return;
		}

		transfer_effeciency = clamp(0.0, 1.0, transfer_effeciency); // Ensure we cannot grow more than they shrink

		float target_scale = get_visual_scale(target);
		float caster_scale = get_visual_scale(caster);

		power *= BASE_POWER * CalcEffeciency(caster, target);

		if (dual_casting) {
			power *= DUAL_CAST_BONUS;
		}

		if (smt) {
			power *= SMT_BONUS;
		}

		if (Runtime::HasPerk(caster, "PerkPart1")) {
			power *= PERK1_BONUS;
		}
		if (Runtime::HasPerk(caster, "PerkPart2")) {
			power *= PERK2_BONUS;
		}
		AdjustSizeLimit(0.0010 * target_scale * power);
		float alteration_level_bonus = 0.0332 + caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlteration) * 0.00166 / 160; // 0.0332 is a equivallent to lvl 20 skill
		Steal(target, caster, power, power*alteration_level_bonus, transfer_effeciency);
	}

	inline bool ShrinkToNothing(Actor* caster, Actor* target) {
		const float SHRINK_TO_NOTHING_SCALE = 0.14;
		float target_scale = get_visual_scale(target);
		if (!caster) {
			return false;
		}

		if (target_scale <= SHRINK_TO_NOTHING_SCALE && !Runtime::HasMagicEffect(target,"ShrinkToNothing") && !target->IsPlayerTeammate()) {
			if (!ShrinkToNothingManager::CanShrink(caster, target)) {
				return false;
			}
			ShrinkToNothingManager::Shrink(caster, target);
			AdjustSizeLimit(0.0117);

			auto Cache = Persistent::GetSingleton().GetData(caster);

			if (!Cache) {
				return false;
			}

			if (Runtime::HasPerk(PlayerCharacter::GetSingleton(), "SizeReserve")) {
				Cache->SizeReserve += target_scale/25;
			}
			ConsoleLog::GetSingleton()->Print("%s Was absorbed by %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
			return true;
		}
		return false;
	}

	inline void PrintCrushMessage(Actor* caster, Actor* target, float sizedifference, int random, float instacrushrequirement) {
		if (sizedifference < instacrushrequirement && random < 4) {
			ConsoleLog::GetSingleton()->Print("%s Was crushed by %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		} else if (sizedifference < instacrushrequirement && random >= 4) {
			ConsoleLog::GetSingleton()->Print("%s Got crushed by %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		} else if (sizedifference < instacrushrequirement && random >= 6) {
			ConsoleLog::GetSingleton()->Print("%s Crushed %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		} else if (sizedifference < instacrushrequirement && random >= 8) {
			ConsoleLog::GetSingleton()->Print("%s Was defeated by the feet of %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		} else if (sizedifference >= instacrushrequirement && random < 4) {
			ConsoleLog::GetSingleton()->Print("%s Crushed %s without noticing it", caster->GetDisplayFullName(), target->GetDisplayFullName());
		} else if (sizedifference >= instacrushrequirement && random >= 4) {
			ConsoleLog::GetSingleton()->Print("%s Accidentally Crushed %s", caster->GetDisplayFullName(), target->GetDisplayFullName());
		} else if (sizedifference >= instacrushrequirement && random >= 6) {
			ConsoleLog::GetSingleton()->Print("%s was defeated by feet of %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		} else if (sizedifference >= instacrushrequirement && random >= 8) {
			ConsoleLog::GetSingleton()->Print("%s was turned into mush by the feet of %s", target->GetDisplayFullName(), caster->GetDisplayFullName());
		}
	}

	inline void CrushBonuses(Actor* caster, Actor* target) {
		float target_scale = get_target_scale(target);
		float caster_scale = get_target_scale(caster);
		auto player = PlayerCharacter::GetSingleton();
		float sizedifference = caster_scale/target_scale;
		float instacrushrequirement = 24.0;

		if (Runtime::GetBool("ProtectEssentials") && target->IsEssential()) {
			return;
		}
		int Random = rand() % 8;
		if (Random >= 8 && Runtime::HasPerk(caster, "GrowthPerk")) {
			Runtime::PlaySound("MoanSound",caster, 1.0, 1.0);
		}
		PrintCrushMessage(caster, target, sizedifference, Random, instacrushrequirement);
		bool hasSMT = Runtime::HasMagicEffect(caster, "SmallMassiveThreat");
		if (get_visual_scale(caster) <= 12.0 && !caster->AsActorState()->IsSprinting() && !caster->AsActorState()->IsWalking() && !caster->IsRunning() && !hasSMT || hasSMT && get_visual_scale(caster) <= 12.0) {
			PlayAnimation(caster, "JumpLand");
		}
		auto Cache = Persistent::GetSingleton().GetData(caster); // TODO: Fix this properly
		if (!Cache) {
			return;
		}
		if (caster == player)  {
				bool hasExplosiveGrowth1 = Runtime::HasMagicEffect(caster, "explosiveGrowth1");
				bool hasExplosiveGrowth2 = Runtime::HasMagicEffect(caster, "explosiveGrowth2");
				bool hasExplosiveGrowth3 = Runtime::HasMagicEffect(caster, "explosiveGrowth3");
			if (Runtime::HasPerk(player, "SizeReserve")) {
				Cache->SizeReserve += target_scale/25;
			}
				AdjustSizeLimit(0.0417 * target_scale);
			if (Runtime::HasPerk(caster, "ExtraGrowth") && (hasExplosiveGrowth1 || hasExplosiveGrowth2 || hasExplosiveGrowth3)) {
				auto CrushGrowthStorage = Runtime::GetFloat("CrushGrowthStorage");
				Runtime::SetFloat("CrushGrowthStorage", CrushGrowthStorage + (target_scale/75) / SizeManager::GetSingleton().BalancedMode());
			} 
			// Slowly increase Crush Growth Limit after crushing someone while Growth Spurt is active.
		}
	}
	

	inline void CastTrackSize(Actor* caster, Actor* target) {
		Runtime::CastSpell(caster, target, "TrackSizeSpell");
	}
}
