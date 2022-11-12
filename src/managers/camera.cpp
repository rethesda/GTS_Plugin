#include "managers/camera.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void SetINIFloat(std::string_view name, float value) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			setting->data.f=value; // If float
			ini_conf->WriteSetting(setting);
		}
	}
	float GetINIFloat(std::string_view name) {
		auto ini_conf = INISettingCollection::GetSingleton();
		Setting* setting = ini_conf->GetSetting(name);
		if (setting) {
			return setting->data.f;
		}
		return -1.0;
	}

	void UpdateThirdPerson() {
		auto camera = PlayerCamera::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		if (camera && player) {
			camera->UpdateThirdPerson(player->IsWeaponDrawn());
		}
	}
}

namespace Gts {
	CameraManagerOld& CameraManagerOld::GetSingleton() noexcept {
		static CameraManagerOld instance;
		return instance;
	}

	void CameraManagerOld::SetfOverShoulderPosX(float value) {
		SetINIFloat("fOverShoulderPosX:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderPosX() {
		return GetINIFloat("fOverShoulderPosX:Camera");
	}

	void CameraManagerOld::SetfOverShoulderPosY(float value) {
		SetINIFloat("fOverShoulderPosY:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderPosY() {
		return GetINIFloat("fOverShoulderPosY:Camera");
	}

	void CameraManagerOld::SetfOverShoulderPosZ(float value) {
		SetINIFloat("fOverShoulderPosZ:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderPosZ() {
		return GetINIFloat("fOverShoulderPosZ:Camera");
	}

	void CameraManagerOld::SetfOverShoulderCombatPosX(float value) {
		SetINIFloat("fOverShoulderCombatPosX:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderCombatPosX() {
		return GetINIFloat("fOverShoulderPosCombatX:Camera");
	}

	void CameraManagerOld::SetfOverShoulderCombatPosY(float value) {
		SetINIFloat("fOverShoulderCombatPosY:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderCombatPosY() {
		return GetINIFloat("fOverShoulderCombatPosY:Camera");
	}

	void CameraManagerOld::SetfOverShoulderCombatPosZ(float value) {
		SetINIFloat("fOverShoulderCombatPosZ:Camera", value);
	}
	float CameraManagerOld::GetfOverShoulderCombatPosZ() {
		return GetINIFloat("fOverShoulderCombatPosZ:Camera");
	}

	void CameraManagerOld::SetfVanityModeMaxDist(float value) {
		SetINIFloat("fVanityModeMaxDist:Camera", value);
	}
	float CameraManagerOld::GetfVanityModeMaxDist() {
		return GetINIFloat("fVanityModeMaxDist:Camera");
	}

	void CameraManagerOld::SetfVanityModeMinDist(float value) {
		SetINIFloat("fVanityModeMinDist:Camera", value);
	}
	float CameraManagerOld::GetfVanityModeMinDist() {
		return GetINIFloat("fVanityModeMinDist:Camera");
	}

	void CameraManagerOld::SetfMouseWheelZoomSpeed(float value) {
		SetINIFloat("fMouseWheelZoomSpeed:Camera", value);
	}

	void CameraManagerOld::SetfMouseWheelZoomIncrement(float value) {
		SetINIFloat("fMouseWheelZoomIncrement:Camera", value);
	}

	float CameraManagerOld::GetfMouseWheelZoomSpeed() {
		return GetINIFloat("fMouseWheelZoomSpeed:Camera");
	}

	void CameraManagerOld::AdjustSide(bool Reset, bool Right, bool Left) {
		float size = get_visual_scale(PlayerCharacter::GetSingleton());
		if (Reset) {
			this->Side = 1.0;
			//log::info("Side Reset: {}", this->Side);
		}
		if (Left) {
			this->Side -= (1.0 + (size * 0.0125 - 0.0125));
			//log::info("Side Left: {}", this->Side);
		}
		if (Right) {
			this->Side += (1.0 + (size * 0.0125 - 0.0125));
			//log::info("Side Right: {}", this->Side);
		}
	}

	void CameraManagerOld::AdjustUpDown(bool Reset, bool Up, bool Down) {
		float size = get_visual_scale(PlayerCharacter::GetSingleton());
		if (Reset) {
			this->UpDown = 1.0;
			//log::info("UpDown Reset: {}", this->UpDown);
		}
		if (Up) {
			this->UpDown += (1.0 + (size * 0.0125 - 0.0125));
			//log::info("UpDown Decrease: {}", this->UpDown);
		}
		if (Down) {
			this->UpDown -= (1.0 + (size * 0.0125 - 0.0125));
			//log::info("UpDown Increase: {}", this->UpDown);
		}
	}

	void CameraManagerOld::UpdateFirstPerson(bool ImProne) {
		auto player = PlayerCharacter::GetSingleton();
		float scale = get_target_scale(player);
		float ProneOffsetFP = 1.0;
		if (player->IsSneaking() == true && ImProne == true) {
			ProneOffsetFP = clamp(0.25, 20.0, 3.0 * Runtime::GetFloat("ProneOffsetFP"));
		}
		set_fp_scale(player, scale, ProneOffsetFP);
	}

	void CameraManagerOld::ApplyCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne) {
		float cameraYCorrection = 121.0;
		float UpDown = this->UpDown; float Side = this->Side;
		float CalcProne = Runtime::GetFloat("CalcProne");
		float ProneOffsetFP = 1.0;

		SetfOverShoulderPosX((X + Side) * size);
		SetfOverShoulderPosZ((Y + UpDown) * size - cameraYCorrection);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ(((AltY + UpDown) * size) - cameraYCorrection);

		SetfOverShoulderPosX(((X + Side) * size));
		SetfOverShoulderPosZ(((Y + UpDown) * size) - cameraYCorrection);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ((AltY + UpDown) * size - cameraYCorrection);
		if (usingAutoDistance <= 0.0) {
			SetfVanityModeMinDist(MinDistance * size);
			SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
			float ProneCalc = CameraManagerOld::GetfOverShoulderPosZ(); //Utility.getINIFloat("fOverShoulderPosZ:Camera")
			float ProneCalcC = CameraManagerOld::GetfOverShoulderCombatPosZ(); //Utility.getINIFloat("fOverShoulderCombatPosZ:Camera")
			CameraManagerOld::SetfOverShoulderPosZ(ProneCalc * (1.0 - Runtime::GetFloat("CalcProne"))); //Utility.setINIFloat("fOverShoulderPosZ:Camera", ProneCalc * CalcProne2)
			CameraManagerOld::SetfOverShoulderCombatPosZ(ProneCalcC * (1.0 - Runtime::GetFloat("CalcProne"))); //Utility.setINIFloat("fOverShoulderCombatPosZ:Camera", ProneCalcC * CalcProne2)
		}
	}

	void CameraManagerOld::ApplyFeetCameraSettings(float size, float X, float Y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne) {
		float cameraYCorrection2 = 205.0 * (size * 0.33) + 70;
		float CalcProne2 = Runtime::GetFloat("CalcProne");
		float UpDown = this->UpDown; float Side = this->Side;
		float ProneOffsetFP = 1.0;

		CameraManagerOld::SetfOverShoulderPosX(((X + Side) * size));
		CameraManagerOld::SetfOverShoulderPosZ(((Y + UpDown) * size) - cameraYCorrection2);

		SetfOverShoulderCombatPosX((AltX + Side) * size);
		SetfOverShoulderCombatPosZ((AltY + UpDown) * size - cameraYCorrection2);
		if (usingAutoDistance <= 0.0) {
			SetfVanityModeMinDist(MinDistance * size);
			SetfVanityModeMaxDist(MaxDistance * size);
		}

		if (PlayerCharacter::GetSingleton()->IsSneaking() == true && ImProne == true) {
			float ProneCalc = CameraManagerOld::GetfOverShoulderPosZ(); //Utility.getINIFloat("fOverShoulderPosZ:Camera")
			float ProneCalcC = CameraManagerOld::GetfOverShoulderCombatPosZ(); //Utility.getINIFloat("fOverShoulderCombatPosZ:Camera")
			CameraManagerOld::SetfOverShoulderPosZ(ProneCalc * (1.0 - (Runtime::GetFloat("CalcProne")))); //Utility.setINIFloat("fOverShoulderPosZ:Camera", ProneCalc * CalcProne2)
			CameraManagerOld::SetfOverShoulderCombatPosZ(ProneCalcC * (1.0 - Runtime::GetFloat("CalcProne"))); //Utility.setINIFloat("fOverShoulderCombatPosZ:Camera", ProneCalcC * CalcProne2)
		}

	}

	std::string CameraManagerOld::DebugName() {
		return "CameraManagerOld";
	}

	// Run every frame
	void CameraManagerOld::Update() {
		auto player = PlayerCharacter::GetSingleton();
		float size = get_visual_scale(player);
		// Early exit
		//if (!this->CameraTimer.ShouldRunFrame()) {
		//return;
		//}
		if (fabs(size - this->last_scale) <= 1e-4) {
			return;
		}
		this->last_scale = size;

		auto Camera = PlayerCamera::GetSingleton();
		float CameraX = Camera->pos.x;
		float CameraY = Camera->pos.y;
		float CameraZ = Camera->pos.z;


		bool ImProne = false;
		float ScaleMethod = 0.0;
		SizeMethod method = Persistent::GetSingleton().size_method;
		if (method == SizeMethod::ModelScale) {
			ScaleMethod = 0.0;
		} else if (method == SizeMethod::RootScale) {
			ScaleMethod = 1.0;
		} else if (method == SizeMethod::RefScale) {
			ScaleMethod = 2.0;
		}

		float EnableCamera = Runtime::GetInt("EnableCamera");
		float EnableAltCamera = Runtime::GetInt("EnableAltCamera");
		float FeetCamera = Runtime::GetInt("FeetCamera");
		float usingAutoDistance = Runtime::GetInt("usingAutoDistance");
		float ImCrouching = Runtime::GetInt("ImCrouching");
		float MinDistance = Runtime::GetFloat("MinDistance");
		float MaxDistance = Runtime::GetFloat("MaxDistance");
		float CameraZoomSpeed = Runtime::GetFloat("CameraZoomSpeed");
		float CameraZoomPrecision = Runtime::GetFloat("CameraZoomPrecision");
		//////////Normal - Prone
		float proneCameraX = Runtime::GetFloat("proneCameraX");
		float proneCameraY = Runtime::GetFloat("proneCameraY");
		float proneCombatCameraX = Runtime::GetFloat("proneCombatCameraX");
		float proneCombatCameraY = Runtime::GetFloat("proneCombatCameraY");
		/////////Normal - Normal
		float cameraX = Runtime::GetFloat("cameraX");
		float cameraY = Runtime::GetFloat("cameraY");
		float combatCameraX = Runtime::GetFloat("combatCameraX");
		float combatCameraY = Runtime::GetFloat("combatCameraY");
		//------------------------------------------------------------------------------------------------------------------------
		/////////Alternate - Prone
		float proneCameraAlternateX = Runtime::GetFloat("proneCameraAlternateX");
		float proneCameraAlternateY = Runtime::GetFloat("proneCameraAlternateY");
		float proneCombatCameraAlternateX = Runtime::GetFloat("proneCombatCameraAlternateX");
		float proneCombatCameraAlternateY = Runtime::GetFloat("proneCameraAlternateY");
		////////Alternate - Normal
		float cameraAlternateX = Runtime::GetFloat("cameraAlternateX");
		float cameraAlternateY = Runtime::GetFloat("cameraAlternateY");
		float combatCameraAlternateX = Runtime::GetFloat("combatCameraAlternateX");
		float combatCameraAlternateY = Runtime::GetFloat("combatCameraAlternateY");


		if (ImCrouching >= 1.0) {
			ImProne = true;
		} else {
			ImProne = false;
		}

		UpdateFirstPerson(ImProne); // Update FP camera

		if (EnableCamera < 1.0) { // If Enable Automatic Camera is disabled.
			return;
		} else if (MinDistance < -200) {
			MinDistance = -200;
		}
		if (MaxDistance < 50) {
			MaxDistance = 50;
		}


		if (EnableAltCamera >= 3.0) { // Adjustment for Feet Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManagerOld::ApplyFeetCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			} else {
				CameraManagerOld::ApplyFeetCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}
		} else if (EnableAltCamera >= 2.0 && EnableAltCamera < 3.0) { // Adjustment for Alternate Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManagerOld::ApplyCameraSettings(size, proneCameraAlternateX, proneCameraAlternateY, proneCombatCameraAlternateX, proneCombatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			} else {
				CameraManagerOld::ApplyCameraSettings(size, cameraAlternateX, cameraAlternateY, combatCameraAlternateX, combatCameraAlternateY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}
		} else if (EnableCamera >= 1.0) { // Regular Camera
			SetfMouseWheelZoomIncrement(CameraZoomPrecision);
			SetfMouseWheelZoomSpeed(CameraZoomSpeed/2);
			if (player->IsSneaking() == true && ImProne == true) {
				CameraManagerOld::ApplyCameraSettings(size, proneCameraX, proneCameraY, proneCombatCameraX, proneCombatCameraY, MinDistance, MaxDistance,usingAutoDistance, ImProne);
			} else {
				CameraManagerOld::ApplyCameraSettings(size, cameraX, cameraY, combatCameraX, combatCameraY, MinDistance, MaxDistance, usingAutoDistance, ImProne);
			}

		}
		UpdateThirdPerson();
	}
}
