#include "magic/effects/explosive_growth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/magic.hpp"
#include "magic/effects/common.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"
#include "timer.hpp"



namespace Gts {
	std::string ExplosiveGrowth::GetName() {
		return "ExplosiveGrowth";
	}

	ExplosiveGrowth::ExplosiveGrowth(ActiveEffect* effect) : Magic(effect) {
		const float GROWTH_1_POWER = 0.00125;
		const float GROWTH_2_POWER = 0.00145;
		const float GROWTH_3_POWER = 0.00175;

		auto base_spell = GetBaseEffect();
		auto& runtime = Runtime::GetSingleton();
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		if (base_spell == runtime.explosiveGrowth1) {
			this->power = GROWTH_1_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.01; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 1.67; // NOLINT
			} else {
				this->grow_limit = 1.34; // NOLINT
			}
		} else if (base_spell == runtime.explosiveGrowth2) {
			this->power = GROWTH_2_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.34; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 2.01; // NOLINT
			} else {
				this->grow_limit = 1.67; // NOLINT
			}
		} else if (base_spell == runtime.explosiveGrowth3) {
			this->power = GROWTH_3_POWER;
			if (caster->HasPerk(runtime.ExtraGrowthMax)) {
				this->grow_limit = 2.67; // NOLINT
				this->power *= 2.0; // NOLINT
			} else if (caster->HasPerk(runtime.ExtraGrowth)) {
				this->grow_limit = 2.34; // NOLINT
			} else {
				this->grow_limit = 2.01; // NOLINT
			}
		}
	}

	bool ExplosiveGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.explosiveGrowth1 || effect == runtime.explosiveGrowth2 || effect == runtime.explosiveGrowth3);
	}

	void ExplosiveGrowth::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
	}

	void ExplosiveGrowth::OnUpdate() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto sizemanager = SizeManager::GetSingleton();
		float AdjustLimit = clamp(1.0, 12.0, runtime.CrushGrowthStorage->value + 1.0);
		float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(caster)/100;
		float GetGrowthSpurt = SizeManager::GetSingleton().GetGrowthSpurt(caster);
		float scale = get_visual_scale(caster);

		float limit = this->grow_limit * Gigantism * AdjustLimit;

		auto HealthRegenPerk = runtime.HealthRegenPerk;
		float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.00075;

		if (caster->HasPerk(HealthRegenPerk)) {
			caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
		}
		
		if (scale < limit) {
			DoGrowth(caster, this->power);
			if (GetGrowthSpurt < limit) {
				SizeManager::GetSingleton().SetGrowthSpurt(caster, limit);
			}
		}

		else if (limit < GetGrowthSpurt) {
			float difference = GetGrowthSpurt - limit;
			DoShrink(caster, difference/100);
			log::info("Difference is: {}", difference);
		}
		log::info("Growth Spurt: {}, Total Limit is: {}", GetGrowthSpurt, limit);
	}

	void ExplosiveGrowth::OnFinish() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		SizeManager::GetSingleton().SetGrowthSpurt(caster, 0.0);
	}



	void ExplosiveGrowth::DoGrowth(Actor* actor, float value) {
			Grow(actor, value, 0.0); // Grow
			auto runtime = Runtime::GetSingleton();

			GrowthTremorManager::GetSingleton().CallRumble(actor, actor, 1.0);
			if (this->timerSound.ShouldRunFrame()) {
				PlaySound(runtime.xlRumbleL, actor, this->power/20, 0.0);
			}
			if (this->timer.ShouldRun()) {
				auto GrowthSound = runtime.growthSound;
				float Volume = clamp(0.12, 2.0, get_visual_scale(actor)/4);
				PlaySound(GrowthSound, actor, Volume, 0.0);
			}
		}

		void ExplosiveGrowth::DoShrink(Actor* actor, float value) {
			Grow(actor, -value, 0.0); // Grow
			auto runtime = Runtime::GetSingleton();

			GrowthTremorManager::GetSingleton().CallRumble(actor, actor, 1.0);
			if (this->timerSound.ShouldRunFrame()) {
				PlaySound(runtime.xlRumbleL, actor, this->power/20, 0.0);
			}
			if (this->timer.ShouldRun()) {
				auto GrowthSound = runtime.shrinkSound;
				float Volume = clamp(0.12, 2.0, get_visual_scale(actor)/2);
				PlaySound(GrowthSound, actor, Volume, 0.0);
			}
		}

		void ExplosiveGrowth::Start() {
			auto sizemanager = SizeManager::GetSingleton();
			auto caster = PlayerCharacter::GetSingleton();
			sizemanager.SetEnchantmentBonus(caster, 0.0);
			sizemanager.SetGrowthSpurt(caster, 0.0);
		}
	}
	
