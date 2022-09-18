#pragma once


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class InputManager
	{
	public:

        [[nodiscard]] static InputManager& GetSingleton() noexcept;
    
        inline string GetActionString() {
            auto KeyPress = RE::ControlMap::GetSingleton()->inputKey;
            return KeyPress;
            //auto Action = RE::UserEvents::GetSingleton()->activate;
            //return Action;
        }
    };
}