#include "ControlsManager.h"
#include "TimelineManager.h"
#include "APIManager.h"
#include "Offsets.h"

namespace FCSE {

    RE::BSEventNotifyControl ControlsManager::ProcessEvent(RE::InputEvent* const* a_event, RE::BSTEventSource<RE::InputEvent*>*) {

        if (!a_event || RE::UI::GetSingleton()->GameIsPaused()) {
            return RE::BSEventNotifyControl::kContinue;
        }

        for (auto* event = *a_event; event; event = event->next) {
            if (event->eventType == RE::INPUT_EVENT_TYPE::kButton) {
                auto* buttonEvent = static_cast<RE::ButtonEvent*>(event);
                if (!buttonEvent || !buttonEvent->IsDown()) {
                    continue;
                }
                
                int ret;

                const char* relativePath = "SKSE/Plugins/FCSE_CameraPath.yaml";

                SKSE::PluginHandle handle = SKSE::GetPluginHandle();
                auto timelineID = TimelineManager::GetSingleton().GetTimelineID();

                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 2) {
                    if (APIs::FCFW->IsPlaybackPaused(handle, timelineID)) {
                        ret = APIs::FCFW->ResumePlayback(handle, timelineID);
                    } else {
                        ret = APIs::FCFW->PausePlayback(handle, timelineID);
                    }
                } else if (key == 3) {
                    ret = APIs::FCFW->StopPlayback(handle, timelineID);
                } else if (key == 4) {
                    APIs::FCFW->AllowUserRotation(handle, timelineID, !APIs::FCFW->IsUserRotationAllowed(handle, timelineID));
                } else if (key == 5) {
                    RE::TESObjectREFR* reference = nullptr;
                    auto* form = RE::TESForm::LookupByID(0xd8c58);
                    reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                    if (reference) {
                        bool isOffsetRelative = true;

                        auto headPos = GetTargetPoint(reference->As<RE::Actor>());
                        RE::NiPoint3 offset;
                        if (headPos) {
                            offset = headPos->world.translate - reference->GetPosition();
                            offset.y += 20.f;
                        }
                        RE::BSTPoint2<float> rotOffset = {0.f, 0.f};

                        ret = APIs::FCFW->AddTranslationPointAtCamera(handle, timelineID, 0.0f, true, true);
                        ret = APIs::FCFW->AddRotationPointAtCamera(handle, timelineID, 0.f, true, true);
                        ret = APIs::FCFW->AddRotationPointAtRef(handle, timelineID, 0.5f, reference, rotOffset, false, true, true);
                        ret = APIs::FCFW->AddRotationPointAtRef(handle, timelineID, 1.5f, reference, rotOffset, false, true, true);
                        ret = APIs::FCFW->AddTranslationPointAtRef(handle, timelineID, 2.f, reference, offset, isOffsetRelative, true, true);
                        ret = APIs::FCFW->AddRotationPointAtRef(handle, timelineID, 2.f, reference, rotOffset, isOffsetRelative, true, true);
                        ret = APIs::FCFW->AddTranslationPointAtRef(handle, timelineID, 8.f, reference, offset, isOffsetRelative, true, true);
                        ret = APIs::FCFW->AddRotationPointAtRef(handle, timelineID, 8.f, reference, rotOffset, isOffsetRelative, true, true);
                        ret = APIs::FCFW->AddRotationPointAtRef(handle, timelineID, 9.f, RE::PlayerCharacter::GetSingleton(), rotOffset, false, true, true);
                        ret = APIs::FCFW->AddTranslationPointAtCamera(handle, timelineID, 10.0f, true, true);
                        ret = APIs::FCFW->AddRotationPointAtCamera(handle, timelineID, 10.f, true, true);
                        
                        log::info("Created timeline {} with reference tracking", timelineID);
                        ret = APIs::FCFW->StartPlayback(handle, timelineID, 1.0f, false, false, false, 0.0f);
                    }
                } else if (key == 6) {
                    ret = APIs::FCFW->ClearTimeline(handle, timelineID);
                } else if (key == 7) {
                    ret = APIs::FCFW->StartPlayback(handle, timelineID, 1.0f, false, false, false, 0.0f);
                } else if (key == 8) {
                    ret = APIs::FCFW->StartRecording(handle, timelineID);
                } else if (key == 9) {
                    ret = APIs::FCFW->StopRecording(handle, timelineID);
                } else if (key == 10) {
                    RE::DebugNotification("Exporting camera path...");
                    ret = APIs::FCFW->ExportTimeline(handle, timelineID, relativePath);
                } else if (key == 11) {
                    RE::DebugNotification("Importing camera path...");
                    ret = APIs::FCFW->AddTimelineFromFile(handle, timelineID, relativePath);
                } else if (key == 20) { // T
                    ret = TimelineManager::GetSingleton().RegisterTimeline();
                } else if (key == 21) { // Y
                    ret = TimelineManager::GetSingleton().UnregisterTimeline();
                } else if (key == 22) { // U
                    ret = TimelineManager::GetSingleton().CycleUp();
                } else if (key == 35) { // H
                    ret = TimelineManager::GetSingleton().CycleDown();
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }

    RE::NiPointer<RE::NiAVObject> ControlsManager::GetTargetPoint(RE::Actor* a_actor) {
        RE::NiPointer<RE::NiAVObject> targetPoint = nullptr;

        if (!a_actor) {
            return nullptr;
        }

        auto race = a_actor->GetRace();
        if (!race) {
            return nullptr;
        }

        RE::BGSBodyPartData* bodyPartData = race->bodyPartData;
        if (!bodyPartData) {
            return nullptr;
        }

        auto actor3D = a_actor->Get3D2();
        if (!actor3D) {
            return nullptr;
        }
    
        RE::BGSBodyPart* bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kHead];
        if (!bodyPart) {
            bodyPart = bodyPartData->parts[RE::BGSBodyPartDefs::LIMB_ENUM::kTotal];
        }
        if (bodyPart) {
            targetPoint = RE::NiPointer<RE::NiAVObject>(NiAVObject_LookupBoneNodeByName(actor3D, bodyPart->targetName, true));
        }

        return targetPoint;
    }
} // namespace FCSE
