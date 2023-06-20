#include "colliders/charcontroller.hpp"
#include "colliders/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	CharContData::CharContData(bhkCharacterController* charCont) {
		this->ReInit(charCont);
	}

	void CharContData::ReInit(bhkCharacterController* charCont) {
		this->charCont = charCont;
		this->rbs.clear();
		this->phantoms.clear();

		if (!charCont) {
			return;
		}


		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charCont);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);
				if (hkpObject) {
					// This one appears to be active during combat.
					// Maybe used for sword swing collision detection
					for (auto phantom: hkpObject->phantoms) {
						this->AddPhantom(phantom);
					}

					// This is the actual shape
					if (hkpObject->shapePhantom) {
						this->AddPhantom(phantom);
					}
				}
			}
		} else if (charRigidBodyController) {
			// NPCs seem to use rigid body ones
			auto& characterRigidBody = charRigidBodyController->characterRigidBody;
			hkReferencedObject* refObject = characterRigidBody.referencedObject.get();
			if (refObject) {
				hkpCharacterRigidBody* hkpObject = skyrim_cast<hkpCharacterRigidBody*>(refObject);
				if (hkpObject) {
					if (hkpObject->m_character) {
						this->AddRB(shape);
					}
				}
			}
		}
	}

	void CharContData::AddRB(hkpRigidBody* rb) {
		if (!rb) {
			return;
		}
    this->rbs.try_emplace(rb, hkRefPtr(rb));
	}
  void CharContData::AddPhantom(hkpPhantom* phantom) {
		if (!phantom) {
			return;
		}
    this->phantoms.try_emplace(phantom, hkRefPtr(phantom));
	}
}
