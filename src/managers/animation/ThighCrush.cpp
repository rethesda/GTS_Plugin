// Animation: ThighCrush
//  - Stages
//    - "GTStosit",                     // [0] Start air rumble and camera shake
//    - "GTSsitloopenter",              // [1] Sit down completed
//    - "GTSsitloopstart",              // [2] enter sit crush loop
//    - "GTSsitloopend",                // [3] unused
//    - "GTSsitcrushlight_start",       // [4] Start Spreading legs
//    - "GTSsitcrushlight_end",         // [5] Legs fully spread
//    - "GTSsitcrushheavy_start",       // [6] Start Closing legs together
//    - "GTSsitcrushheavy_end",         // [7] Legs fully closed
//    - "GTSsitloopexit",               // [8] stand up, small air rumble and camera shake
//    - "GTSstandR",                    // [9] feet collides with ground when standing up
//    - "GTSstandL",                    // [10]
//    - "GTSstandRS",                   // [11] Silent impact of right feet
//    - "GTStoexit",                    // [12] Leave animation, disable air rumble and such

#include "managers/animation/ThighCrush.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";
	const std::string_view RSound = "lFootstepR";
	const std::string_view LSound = "lFootstepL";

	const std::vector<std::string_view> BODY_RUMBLE_NODES = { // used for body rumble
		"NPC COM [COM ]",
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	const std::vector<std::string_view> LEG_RUMBLE_NODES = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L PreRearCalf",
		"NPC R PreRearCalf",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	void DoEffects(Actor* giant, float modifier, FootEvent kind, std::string_view node) {
		auto& footstep = FootStepManager::GetSingleton();
		auto& explosion = ExplosionManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * modifier,
			.effective_scale = get_effective_scale(giant),
			.nodes = find_node(giant, node),
		};
		explosion.OnImpact(impact_data);
		footstep.OnImpact(impact_data);
	}

	void StartLegRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartBodyRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: BODY_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopLegRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: LEG_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			Rumble::Stop(rumbleName, &actor);
		}
	}

	void GTStosit(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.10, 0.10);
		data.stage = 1;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTStosit");
	}

	void GTSsitloopenter(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StartLegRumble("ThighCrush", data.giant, 0.12 * speed, 0.10);
		data.disableHH = true;
		data.stage = 2;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitloopenter");
	}

	void GTSsitloopstart(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StopLegRumble("ThighCrush", data.giant);
		data.currentTrigger = 1;
		data.stage = 3;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitloopenter");
	}

	void GTSsitloopend(AnimationEventData& data) {
		// Nothing
		data.stage = 4;
	}

	void GTSsitcrushlight_start(AnimationEventData& data) {
		StartLegRumble("ThighCrush", data.giant, 0.18, 0.12);
		data.stage = 5;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitcrushlight_start");
	}

	void GTSsitcrushlight_end(AnimationEventData& data) {
		data.currentTrigger = 2;

		data.canEditAnimSpeed = true;
		StartLegRumble("ThighCrush", data.giant, 0.22, 0.20);
		data.stage = 6;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitcrushlight_end");
	}

	void GTSsitcrushheavy_start(AnimationEventData& data) {
		StartLegRumble("ThighCrush", data.giant, 0.25, 0.20);
		data.stage = 5;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitcrushheavy_start");
	}

	void GTSsitcrushheavy_end(AnimationEventData& data) {
		data.currentTrigger = 2;

		StartLegRumble("ThighCrush", data.giant, 0.35, 0.25);
		data.stage = 6;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitcrushlight_end");
	}

	void GTSsitloopexit(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;

		data.disableHH = false;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;

		StopLegRumble("ThighCrush", data.giant);
		StartBodyRumble("BodyRumble", data.giant, 0.15, 0.12);
		data.stage = 8;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSsitloopexit");
	}

	void GTSstandR(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		float volume = scale * 0.10 * speed;
		StopLegRumble("ThighCrush", data.giant);

		Rumble::Once("ThighCrushStompR", &data.giant, volume * 4, 0.10, RNode);
		DoEffects(&data.giant, 0.75, FootEvent::Right, RNode);
		data.stage = 9;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSstandR");
	}

	void GTSstandL(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StopLegRumble("ThighCrush", data.giant);

		float volume = scale * 0.10 * speed;

		Rumble::Once("ThighCrushStompL", &data.giant, volume * 4, 0.10, LNode);
		DoEffects(&data.giant, 0.75, FootEvent::Left, LNode);
		data.stage = 9;
		//ConsoleLog::GetSingleton()->Print("ThighCrush: GTSstandL");
	}

	void GTSstandRS(AnimationEventData& data) {
		float scale = get_visual_scale(data.giant);
		float speed = data.animSpeed;
		StopLegRumble("ThighCrush", data.giant);

		float volume = scale * 0.05 * speed;

		Rumble::Once("ThighCrushStompR", &data.giant, volume * 4, 0.10, RNode);
		DoEffects(&data.giant, 0.40, FootEvent::Right, RNode);
		data.stage = 9;
	}
	void GTSBEH_Next(AnimationEventData& data) {
		// Inbetween stages
		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
	}
	void GTStoexit(AnimationEventData& data) {
		// Going to exit
		StopLegRumble("ThighCrush", data.giant);
		StopLegRumble("BodyRumble", data.giant);
		StopLegRumble("ThighCrushStompR", data.giant);
		StopLegRumble("ThighCrushStompL", data.giant);
	}
	void GTSBEH_Exit(AnimationEventData& data) {
		// Final exit
		data.stage = 0;
	}

	void ThighCrushEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopEnter", player);
		log::info("Triggering Stage 0");
	}

	void ThighCrushKillEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopAttack", player);
	}

	void ThighCrushSpareEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopExit", player);
	}

	// To Sermit: This seems to be the same as ThighCrushSpareEvent
	//  except it is on the `w` key instead Can we just use the RMB?
	void ThighCrushExitEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("ThighLoopExit", player);
	}
}

namespace Gts
{
	void AnimationThighCrush::RegisterEvents() {
		AnimationManager::RegisterEvent("GTStosit", "ThighCrush", GTStosit);
		AnimationManager::RegisterEvent("GTSsitloopenter", "ThighCrush", GTSsitloopenter);
		AnimationManager::RegisterEvent("GTSsitloopstart", "ThighCrush", GTSsitloopstart);
		AnimationManager::RegisterEvent("GTSsitloopend", "ThighCrush", GTSsitloopend);
		AnimationManager::RegisterEvent("GTSsitcrushlight_start", "ThighCrush", GTSsitcrushlight_start);
		AnimationManager::RegisterEvent("GTSsitcrushlight_end", "ThighCrush", GTSsitcrushlight_end);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_start", "ThighCrush", GTSsitcrushheavy_start);
		AnimationManager::RegisterEvent("GTSsitcrushheavy_end", "ThighCrush", GTSsitcrushheavy_end);
		AnimationManager::RegisterEvent("GTSsitloopexit", "ThighCrush", GTSsitloopexit);
		AnimationManager::RegisterEvent("GTSstandR", "ThighCrush", GTSstandR);
		AnimationManager::RegisterEvent("GTSstandL", "ThighCrush", GTSstandL);
		AnimationManager::RegisterEvent("GTSstandRS", "ThighCrush", GTSstandRS);
		AnimationManager::RegisterEvent("GTStoexit", "ThighCrush", GTStoexit);
		AnimationManager::RegisterEvent("GTSBEH_Next", "ThighCrush", GTSBEH_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "ThighCrush", GTSBEH_Exit);

		InputManager::RegisterInputEvent("ThighCrush", ThighCrushEvent);
		InputManager::RegisterInputEvent("ThighCrushKill", ThighCrushKillEvent);
		InputManager::RegisterInputEvent("ThighCrushSpare", ThighCrushSpareEvent);
		InputManager::RegisterInputEvent("ThighCrushExit", ThighCrushExitEvent);
	}

	void AnimationThighCrush::RegisterTriggers() {
		AnimationManager::RegisterTriggerWithStages("ThighCrush", "ThighCrush", {"GTSBeh_TriggerSitdown", "GTSBeh_StartThighCrush", "GTSBeh_LeaveSitdown"});
		AnimationManager::RegisterTrigger("ThighLoopEnter", "ThighCrush", "GTSBeh_TriggerSitdown");
		AnimationManager::RegisterTrigger("ThighLoopAttack", "ThighCrush", "GTSBeh_StartThighCrush");
		AnimationManager::RegisterTrigger("ThighLoopExit", "ThighCrush", "GTSBeh_LeaveSitdown");
		AnimationManager::RegisterTrigger("ThighLoopFull", "ThighCrush", "GTSBeh_ThighAnimationFull");
	}
}
