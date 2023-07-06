#include "managers/footstep.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"

#include "scale/scale.hpp"
#include "managers/modevent.hpp"
#include "node.hpp"
#include "data/runtime.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	struct VolumeParams {
		float a;
		float k;
		float n;
		float s;
	};

	float volume_function(float scale, const VolumeParams& params) {
		float k = params.k;
		float a = params.a;
		float n = params.n;
		float s = params.s;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		return k*pow(s*(scale-a), n);
	}

	float frequency_function(float scale, const VolumeParams& params) {
		float a = params.a;
		return soft_core(scale, 0.01, 1.0, 1.0, a, 0.0)*0.5+0.5;
	}

	float falloff_function(NiAVObject* source) {
		if (source) {
			float distance_to_camera = unit_to_meter(get_distance_to_camera(source));
			// Camera distance based volume falloff
			return soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0, 0.0);
		}
		return 1.0;
	}

	BSSoundHandle get_sound(NiAVObject* foot, const float& scale, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, std::string_view tag) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {

			float volume = volume_function(scale, params);
			float frequency = frequency_function(scale, params);
			float falloff = falloff_function(foot);
			float intensity = volume * falloff;

			if (intensity > 1e-5) {
				// log::trace("  - Playing {} with volume: {}, falloff: {}, intensity: {}", tag, volume, falloff, intensity);
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				result.SetVolume(intensity);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_lFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("lFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("lFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_lJumpLand_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::JumpLand:
				return Runtime::GetSound("lJumpLand");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlRumble_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlRumbleL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlRumbleR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xlRumbleR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlSprint_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xlSprintL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xlSprintR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xlSprintR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xxlFootstep_sounddesc(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xxlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xxlFootstepR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xxlFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_footstep_highheel(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xxlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xxlFootstepR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xxlFootstepR");
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_footstep_normal(const FootEvent& foot_kind) {
		switch (foot_kind) {
			case FootEvent::Left:
			case FootEvent::Front:
				return Runtime::GetSound("xxlFootstepL");
				break;
			case FootEvent::Right:
			case FootEvent::Back:
				return Runtime::GetSound("xxlFootstepR");
				break;
			case FootEvent::JumpLand:
				return Runtime::GetSound("xxlFootstepR");
				break;
		}
		return nullptr;
	}
}
namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	std::string FootStepManager::DebugName() {
		return "FootStepManager";
	}

	void FootStepManager::OnImpact(const Impact& impact) {
		if (impact.actor) {
			auto player = PlayerCharacter::GetSingleton();
			auto actor = impact.actor;
			float scale = impact.scale;
			if (actor->formID != 0x14) {
				float sizedifference = ((get_visual_scale(actor)/get_visual_scale(player)));
				scale = sizedifference;
			}
			if (actor->formID == 0x14 && HasSMT(actor)) {
				scale *= 2.25;
			}
			float sprint_factor = 1.0;
			bool LegacySounds = true;
			bool sprinting = false;
			bool WearingHighHeels = HighHeelManager::IsWearingHH(actor);
			if (scale > 1.2 && !actor->AsActorState()->IsSwimming()) {
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;

				FootEvent foot_kind = impact.kind;
				
				if (actor->AsActorState()->IsSprinting()) { // Sprinting makes you sound bigger
					sprint_factor = 1.25;
					scale *= sprint_factor; 
					start_xl = 9.8;
					start_xxl = 16.0;
					sprinting = true;
				}
				if (actor->AsActorState()->IsWalking()) {
					scale *= 0.65; // Walking makes you sound quieter
				}
				if (actor->IsSneaking()) {
					scale *= 0.55; // Sneaking makes you sound quieter
				}
				if (actor->formID == 0x14 && IsFirstPerson()) { // Footsteps are quieter when in first person
					scale *= 0.70;
				}

				if (foot_kind == FootEvent::JumpLand) {
					scale *= 1.2; // Jumping makes you sound bigger
					start_xl = 7.8;
					start_xxl = 14.0;
				}
				if (Runtime::GetBool("EnableGiantSounds")) {
					for (NiAVObject* foot: impact.nodes) {
						if (!LegacySounds && WearingHighHeels) { // Play high heel sounds
							FootStepManager::PlayHighHeelSounds(foot, foot_kind, scale * 1.15, sprint_factor, sprinting);
							return;
						} else if (!LegacySounds && !WearingHighHeels) { // Play non HH sounds
							FootStepManager::PlayNormalSounds(foot, foot_kind, scale, sprint_factor, sprinting);
							return;
						} else { // Play old sounds
							FootStepManager::PlayLegacySounds(foot, foot_kind, scale, start_l, start_xl, start_xxl);
							return;
						}
					}
				}
			}
		}
	}

	void FootStepManager::PlayLegacySounds(NiAVObject* foot, FootEvent foot_kind, float scale, float start_l, float start_xl, float start_xxl) {
		BSSoundHandle lFootstep    = get_sound(foot, scale, get_lFootstep_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Footstep");
		BSSoundHandle lJumpLand    = get_sound(foot, scale, get_lJumpLand_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Jump");

		BSSoundHandle xlFootstep   = get_sound(foot, scale, get_xlFootstep_sounddesc(foot_kind),  VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL: Footstep");
		BSSoundHandle xlRumble     = get_sound(foot, scale, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Rumble");
		BSSoundHandle xlSprint     = get_sound(foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Sprint");

		BSSoundHandle xxlFootstepL = get_sound(foot, scale, get_xxlFootstep_sounddesc(foot_kind), VolumeParams { .a = start_xxl,           .k = 0.6,  .n = 0.5, .s = 1.0}, "XXL Footstep");
		if (lFootstep.soundID != BSSoundHandle::kInvalidID) {
			lFootstep.Play();
		}
		if (lJumpLand.soundID != BSSoundHandle::kInvalidID) {
			lJumpLand.Play();
		}
		if (xlFootstep.soundID != BSSoundHandle::kInvalidID) {
			xlFootstep.Play();
		}
		if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
			xlRumble.Play();
		}
		if (xlSprint.soundID != BSSoundHandle::kInvalidID) {
			xlSprint.Play();
		}
		if (xxlFootstepL.soundID != BSSoundHandle::kInvalidID) {
			xxlFootstepL.Play();
		}
	}

	void FootStepManager::PlayHighHeelSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting) {
		BSSoundHandle Footstep_1  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 1.2,           .k = 0.6,  .n = 0.7, .s = 1.0}, "x1 Footstep");
		BSSoundHandle Footstep_2  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 2.0,           .k = 0.6,  .n = 0.7, .s = 1.0}, "x2 Footstep");
		BSSoundHandle Footstep_4  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 4.0,           .k = 0.65, .n = 0.5, .s = 1.0}, "x4 Footstep");
		BSSoundHandle Footstep_8  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 8.0,           .k = 0.65, .n = 0.5, .s = 1.0}, "x8 Footstep");
		BSSoundHandle Footstep_16 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 16.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x16 Footstep");
		BSSoundHandle Footstep_32 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 32.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x32 Footstep");
		BSSoundHandle Footstep_64 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 64.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x64 Footstep");
		BSSoundHandle Footstep_96 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 96.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x96 Footstep");

		if (Footstep_1.soundID != BSSoundHandle::kInvalidID) {
			Footstep_1.Play();
		} if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		} if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		} if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		} if (Footstep_16.soundID != BSSoundHandle::kInvalidID) {
			Footstep_16.Play();
		} if (Footstep_32.soundID != BSSoundHandle::kInvalidID) {
			Footstep_32.Play();
		} if (Footstep_64.soundID != BSSoundHandle::kInvalidID) {
			Footstep_64.Play();
		} if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
	}

	void FootStepManager::PlayNormalSounds(NiAVObject* foot, FootEvent foot_kind, float scale, float sprint, bool sprinting) {
		BSSoundHandle Footstep_1  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 1.2,           .k = 0.6,  .n = 0.7, .s = 1.0}, "x1 FootstepNormal");
		BSSoundHandle Footstep_2  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 2.0,           .k = 0.6,  .n = 0.7, .s = 1.0}, "x2 FootstepNormal");
		BSSoundHandle Footstep_4  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 4.0,           .k = 0.65, .n = 0.5, .s = 1.0}, "x4 FootstepNormal");
		BSSoundHandle Footstep_8  = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 8.0,           .k = 0.65, .n = 0.5, .s = 1.0}, "x8 FootstepNormal");
		BSSoundHandle Footstep_16 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 16.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x16 FootstepNormal");
		BSSoundHandle Footstep_32 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 32.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x32 FootstepNormal");
		BSSoundHandle Footstep_64 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 64.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x64 FootstepNormal");
		BSSoundHandle Footstep_96 = get_sound(foot, scale, get_footstep_highheel(foot_kind),  VolumeParams { .a = 96.0,          .k = 0.65, .n = 0.5, .s = 1.0}, "x96 FootstepNormal");

		if (Footstep_1.soundID != BSSoundHandle::kInvalidID) {
			Footstep_1.Play();
		} if (Footstep_2.soundID != BSSoundHandle::kInvalidID) {
			Footstep_2.Play();
		} if (Footstep_4.soundID != BSSoundHandle::kInvalidID) {
			Footstep_4.Play();
		} if (Footstep_8.soundID != BSSoundHandle::kInvalidID) {
			Footstep_8.Play();
		} if (Footstep_16.soundID != BSSoundHandle::kInvalidID) {
			Footstep_16.Play();
		} if (Footstep_32.soundID != BSSoundHandle::kInvalidID) {
			Footstep_32.Play();
		} if (Footstep_64.soundID != BSSoundHandle::kInvalidID) {
			Footstep_64.Play();
		} if (Footstep_96.soundID != BSSoundHandle::kInvalidID) {
			Footstep_96.Play();
		}
	}

	void FootStepManager::PlayNormalSounds(NiAVObject* foot, FootEvent foot_kind, float scale) {
	}
}
