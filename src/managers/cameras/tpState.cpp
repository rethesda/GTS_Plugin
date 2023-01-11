#include "managers/cameras/tpState.hpp"
#include "managers/cameras/camutil.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;

namespace Gts {
	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffset(const NiPoint3& cameraPos) {
		NiPoint3 pos = NiPoint3();
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto scale = get_visual_scale(player);
			auto boneTarget = this->GetBoneTarget();
			if (!boneTarget.bonesNames.empty()) {
				auto player = PlayerCharacter::GetSingleton();
				if (player) {
					auto rootModel = player->Get3D(false);
					if (rootModel) {
						auto transform = rootModel->world.Invert();

						NiPoint3 lookAt = CompuleLookAt(boneTarget.zoomScale);
						NiPoint3 localLookAt = transform*lookAt;
						this->smoothScale.target = scale;
						pos += localLookAt * -1 * this->smoothScale.value;

						std::vector<NiAVObject*> bones = {};
						for (auto bone_name: boneTarget.boneNames) {
							auto node = find_node(player, bone_name);
							if (node) {
								bones.push_back(node);
							} else {
								log::error("Bone not found for camera target: {}", bone_name);
							}
						}

						NiPoint3 bonePos = NiPoint3();
						auto bone_count = bones.size();
						for (auto bone: bones) {
							auto localPos = transform * (bone->world * NiPoint3());
							bonePos += localPos * (1.0/bone_count);
						}
						smoothedBonePos.target = bonePos;
						pos += smoothedBonePos.value;
					}
				}
			}
		}
		return pos;
	}

	NiPoint3 ThirdPersonCameraState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) {
		NiPoint3 pos = this->GetPlayerLocalOffset(cameraPos);
		auto player = PlayerCharacter::GetSingleton();
		if (player) {
			auto scale = get_visual_scale(player);
			pos += this->ProneAdjustment(cameraPos)*scale;
		}
		return pos;
	}

	BoneTarget ThirdPersonCameraState::GetBoneTarget() {
		return BoneTarget();
	}

	NiPoint3 ThirdPersonCameraState::ProneAdjustment(const NiPoint3& cameraPos) {
		float proneFactor = 1.0 - Runtime::GetFloat("CalcProne");
		NiPoint3 result = NiPoint3();

		result.z = -cameraPos.z * proneFactor;
		return result;
	}
}
