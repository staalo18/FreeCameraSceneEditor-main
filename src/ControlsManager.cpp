#include "ControlsManager.h"
#include "TimelineManager.h"

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
                
                const char* relativePath = "SKSE/Plugins/FCSE_CameraPath.ini";

                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 2) {
                    if (FCSE::TimelineManager::GetSingleton().IsPlaybackPaused()) {
                        FCSE::TimelineManager::GetSingleton().ResumePlayback();
                    } else {
                        FCSE::TimelineManager::GetSingleton().PausePlayback();
                    }
                } else if (key == 3) {
                        FCSE::TimelineManager::GetSingleton().StopPlayback();
                } else if (key == 4) {
                     FCSE::TimelineManager::GetSingleton().AllowUserRotation(!FCSE::TimelineManager::GetSingleton().IsUserRotationAllowed());
                } else if (key == 5) {
                    RE::TESObjectREFR* reference = nullptr;
                    auto* form = RE::TESForm::LookupByID(0xd8c56);
                    reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                    if (reference) {
                        auto& timelineManager = FCSE::TimelineManager::GetSingleton();
                        bool isOffsetRelative = true;
                        timelineManager.ClearTimeline();
                        RE::NiPoint3 camPos = _ts_SKSEFunctions::GetCameraPos();
                        RE::NiPoint3 camRot = _ts_SKSEFunctions::GetCameraRotation();
                        float offsetY = 30.f;
                        float offsetZ = 120.f;

                        timelineManager.AddTranslationPoint(0.f, camPos.x, camPos.y, camPos.z, true, true, 2);
                        timelineManager.AddRotationPoint(0.f, camRot.x, camRot.z, true, true, 2);
                        timelineManager.AddTranslationPointAtRef(2.f, reference, 0.f, offsetY, offsetZ, isOffsetRelative, true, true, 2);
                        timelineManager.AddRotationPointAtRef(2.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, 2);
                        timelineManager.AddTranslationPointAtRef(8.f, reference, 0.f, offsetY, offsetZ, isOffsetRelative, true, true, 2);
                        timelineManager.AddRotationPointAtRef(8.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, 2);
                        timelineManager.AddTranslationPoint(10.f, camPos.x, camPos.y, camPos.z, true, true, 2);
                        timelineManager.AddRotationPoint(10.f, camRot.x, camRot.z, true, true, 2);

                        timelineManager.StartPlayback();
                    }
                } else if (key == 6) {
                    FCSE::TimelineManager::GetSingleton().ClearTimeline();
                } else if (key == 7) {
                    FCSE::TimelineManager::GetSingleton().StartPlayback(); //(1.0f, false, false, false, 12.0f);
                } else if (key == 8) {
                    FCSE::TimelineManager::GetSingleton().StartRecording();
                } else if (key == 9) {
                    FCSE::TimelineManager::GetSingleton().StopRecording();
                } else if (key == 10) {
                    RE::DebugNotification("Exporting camera path...");
                    FCSE::TimelineManager::GetSingleton().ExportTimeline(relativePath);
                } else if (key == 11) {
                    RE::DebugNotification("Importing camera path...");
                    FCSE::TimelineManager::GetSingleton().AddTimelineFromFile(relativePath);
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
