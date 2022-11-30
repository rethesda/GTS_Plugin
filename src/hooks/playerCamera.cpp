#include "hooks/playerCamera.hpp"
#include "data/plugin.hpp"
#include "util.hpp"
#include "managers/altcamera.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCamera::Hook() {
		logger::info("Hooking PlayerCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCamera[0] };
		_Update = Vtbl.write_vfunc(0x02, Update);
	}

	void Hook_PlayerCamera::Update(PlayerCamera* a_this) {
		log::info("Hook_PlayerCamera::Update");
		_Update(a_this);
		CameraManager::GetSingleton().Update();
	}
}
