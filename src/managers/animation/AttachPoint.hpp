#pragma once
#include "scale/scale.hpp"
#include "node.hpp"

using namespace RE;

namespace Gts {

  Actor* GetActorPtr(Actor* actor);

  Actor* GetActorPtr(Actor& actor);

  Actor* GetActorPtr(ActorHandle& actor);

  Actor* GetActorPtr(const ActorHandle& actor);

  Actor* GetActorPtr(FormID formId);

  NiPoint3 GetBreastPos(Actor* giant, std::string_view bone_name) {
    NiPoint3 pos = NiPoint3();
		if (giant) {
			auto scale = get_visual_scale(giant);
			auto rootModel = giant->Get3D(false);
      if (rootModel) {
        auto playerTrans = rootModel->world;
        playerTrans.scale = rootModel->parent ? rootModel->parent->world.scale : 1.0;  // Only do translation/rotation
        auto transform = playerTrans.Invert();
        NiPoint3 localLookAt = transform*1.0;
        pos += localLookAt * -1;
        auto node = find_node(giant, bone_name);
        if (node) {
          NiPoint3 bonePos = NiPoint3(); 
          auto localPos = transform * (node->world * NiPoint3());
          bonePos += localPos;
        }
        return bonePos;
      }
    }
	}

  template<typename T, typename U>
  bool AttachTo(T& anyGiant, U& anyTiny, NiPoint3 point) {
    Actor* giant =  GetActorPtr(anyGiant);
    Actor* tiny =  GetActorPtr(anyTiny);

    if (!giant) {
      return false;
    }
    if (!tiny) {
      return false;
    }



    tiny->SetPosition(point, true);

    auto charcont = tiny->GetCharController();
    if (charcont) {
      charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
    }

    return true;
  }
  template<typename T, typename U>
  bool AttachTo(T& anyGiant, U& anyTiny, std::string_view bone_name) {
    Actor* giant = GetActorPtr(anyGiant);
    if (!giant) {
      return false;
    }
    auto bone = find_node(giant, bone_name);
    if (!bone) {
      return false;
    }
    return AttachTo(anyGiant, anyTiny, bone->world.translate);
  }

  template<typename T, typename U>
  bool AttachToObjectA(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "AnimObjectA");
  }

  template<typename T, typename U>
  bool AttachToObjectB(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "AnimObjectB");
  }

  template<typename T, typename U>
  bool AttachToHand(T& anyGiant, U& anyTiny) {
    return AttachTo(anyGiant, anyTiny, "NPC L Finger02 [LF02]");
  }

  template<typename T, typename U>
  bool AttachToClevage(T& anyGiant, U& anyTiny) {
    Actor* giant = GetActorPtr(anyGiant);
    if (!giant) {
      return false;
    }

    auto breastLNode = find_node(giant, "L Breast02");
    if (!breastLNode) {
      return false;
    }
    auto breastL = breastLNode->world.translate;
    auto breastL_Local = GetBreastPos(giant, "L Breast02");

    auto breastRNode = find_node(giant, "R Breast02");
    if (!breastRNode) {
      return false;
    }
    auto breastR = breastRNode->world.translate;
    auto breastR_Local = GetBreastPos(giant, "R Breast02");

    auto spine2Node = find_node(giant, "NPC Spine2 [Spn2]");
    if (!spine2Node) {
      return false;
    }
    auto spine2 = spine2Node->world.translate;

    float forwardAmount = 1.0;

    auto breastForward = ((breastL + breastL_Local) + (breastR + breastR_Local))/2; //((breastL - spine2) + (breastR - spine2))  * forwardAmount / 2 + spine2;
    return AttachTo(anyGiant, anyTiny, breastForward);
  }
}
