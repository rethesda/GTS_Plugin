#include "hooks/havok.hpp"
#include "events.hpp"
#include "data/transient.hpp"

#include "managers/contact.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
  bool DisabledCollision(const hkpCollidable& collidable) {
    auto type = collidable.broadPhaseHandle.type;
    log::info("IsCollisionEnabled: {}", type);
    if (static_cast<RE::hkpWorldObject::BroadPhaseType>(type) == hkpWorldObject::BroadPhaseType::kEntity) {
      log::info("  - obj: {}", collidable.ownerOffset);
      if (collidable.ownerOffset < 0) {
        log::info("  - obj: ownerOffset < 0");
        hkpRigidBody* obj = collidable.GetOwner<hkpRigidBody>();
        log::info("  - collidable.GetOwner<hkpWorldObject>()");
        if (obj) {
          log::info("  - non null");
          auto tesObj = obj->GetUserData();
          log::info("  - obj->GetUserData()");
          if (tesObj) {
            log::info("  - tesObj");
            log::info("  - tesObj: {}", tesObj->GetDisplayFullName());
            auto tranData = Transient::GetSingleton().GetData(tesObj);
            if (tranData) {
              log::info("  - tranData");
              if (tranData->disable_collision) {
                log::info("  - Disabled");
                return true;
              }
            }
          }
        }
      }
    }
    return false;
  }
}

namespace Hooks
{
	void Hook_Havok::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{RELOCATION_ID(38112, 39068)};
		logger::info("Gts applying Havok Hook at {}", hook.address());
		_ProcessHavokHitJobs = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x104, 0xFC), ProcessHavokHitJobs);

    REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_bhkCollisionFilter[1] };
		_IsCollisionEnabled = Vtbl.write_vfunc(0x1, IsCollisionEnabled);
	}

	void Hook_Havok::ProcessHavokHitJobs(void* a1)
	{
		_ProcessHavokHitJobs(a1);

		EventDispatcher::DoHavokUpdate();
	}

  // Credit: FlyingParticle for code on getting the NiAvObject
  //         maxsu. for IsCollisionEnabled idea
  bool Hook_Havok::IsCollisionEnabled(hkpCollidableCollidableFilter* a_this, const hkpCollidable& a_collidableA, const hkpCollidable& a_collidableB) {
    log::info("- IsCollisionEnabled");
    log::info("  - Name: {}", GetRawName(a_this));
    bool result = _IsCollisionEnabled(a_this, a_collidableA, a_collidableB);
    if (result) {
      if (DisabledCollision(a_collidableA) || DisabledCollision(a_collidableB)) {
        result = false;
      }
    }
    return result;
  }
}
