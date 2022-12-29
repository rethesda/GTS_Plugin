#include "managers/RipClothManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	ClothManager& ClothManager::GetSingleton() noexcept {
		static ClothManager instance;
		return instance;
	}

    std::string ClothManager::DebugName() {
		return "ClothManager";
	}

	void ClothManager::CheckRip() {
		auto player = PlayerCharacter::GetSingleton();
        float scale = get_target_scale(player);

        auto feet = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
        auto head = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kHead);
        auto body = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kBody);
        auto hands = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kHands);
        auto forearms = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kForearms);
        auto calves = player->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kCalves);
        int RandomSlot = rand() % 5; // Randomly choose slot to strip

        auto ArmorSlot = feet;

        if (RandomSlot == 0) {
            ArmorSlot = feet;
        }
        else if (RandomSlot == 1) {
            ArmorSlot = head;
        }
        else if (RandomSlot == 2) {
            ArmorSlot = body;
        }
        else if (RandomSlot == 3) {
            ArmorSlot = hands;
        }
        else if (RandomSlot == 4) {
            ArmorSlot = forearms;
        }
        else if (RandomSlot == 5) {
            ArmorSlot = calves;
        }
    
        static Timer timer = Timer(4.5);
        if (timer.ShouldRunFrame()) {
            //log::info("Armor Slot: {}", ArmorSlot);
		    if (!player || scale <= 2.5) {
                log::info("Scale <= 2.5");
			    return;
		    }
             if (Runtime::GetFloat("AllowClothTearing") == 0.0) {
                 log::info("Cloth Tearing Disabled");
                 return; // Abort doing anything if not set to 1
             }
             if (this->clothtearcount >= 5.0) {
                this->clothtearcount = 0.0;
                this->clothtearthreshold = 2.5; // reset stuff
             }
             if (ArmorSlot != nullptr) {
                //log::info("Armor Name: {}", ArmorSlot->GetName());
                log::info("Armor is not nullptr");
             }
             if (scale >= this->clothtearthreshold) {
                log::info("Scale >= threshold");
             }
             if (ArmorSlot != nullptr && scale >= this->clothtearthreshold)
             {
                this->clothtearthreshold += rand() % 75; 
                this->clothtearcount +=1.0;
                ArmorSlot->As<TESObjectREFR>()->UnequipItem(true, ArmorSlot);
                Runtime::PlaySound("ClothTearSound", player, 1.0, 1.0);
                Runtime::PlaySound("MoanSound", player, 1.0, 1.0);
                GrowthTremorManager::GetSingleton().CallRumble(player, player, 2 * scale);
                log::info("Cloth Tearing Success. Threshold: {}, count: {}", this->clothtearthreshold, this->clothtearcount);
            }
	    }
    } 
}
