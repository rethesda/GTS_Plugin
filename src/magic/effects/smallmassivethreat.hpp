#pragma once
#include "magic/magic.hpp"
// Module that handles footsteps


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class SmallMassiveThreat : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual std::string GetName() override;

            static bool StartEffect(EffectSetting* effect);

			inline float SmallMassiveThreat::Augmentation() 

			void OverrideBonus(float Value)

		private: 
			float MovementSpeedBonus = 0.0;
	};
}
