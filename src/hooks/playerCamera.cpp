#include "hooks/playerCamera.hpp"
#include "data/plugin.hpp"

using namespace RE;

namespace Hooks
{
	// BGSImpactManager
	void Hook_PlayerCamera::Hook() {
		logger::info("Hooking PlayerCamera");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_PlayerCamera[0] };
		_SetCameraRoot = Vtbl.write_vfunc(0x01, SetCameraRoot);
		_Update = Vtbl.write_vfunc(0x02, Update);
	}

	void Hook_PlayerCamera::SetCameraRoot(PlayerCamera* a_this, NiPointer<NiNode> a_root) {
		log::info("Player camera SetCameraRoot");
		if (Gts::Plugin::InGame()) {
			log::info("In Game");
			auto player = PlayerCharacter::GetSingleton();
			if (player) {
				log::info("Player exists");
				if (a_root) {
					log::info("Getting name");
					log::trace("  - Node {}", a_root->name);
				}
				log::info("Getting model");
				auto model = player->GetCurrent3D();
				if (model) {
					auto node = model->AsNode();
					if (node) {
						log::info("Alternative Set");
						_SetCameraRoot(a_this, NiPointer<NiNode>(node));
						return;
					}
				}
			}
		}
		log::info("Nomral Set");
		_SetCameraRoot(a_this, a_root);
	}

	void Hook_PlayerCamera::Update(PlayerCamera* a_this) {
		log::info("Player camera Update");
		_Update(a_this);
	}
}
