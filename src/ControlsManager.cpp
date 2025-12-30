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
                        float offsetX;
                        float offsetY;
                        float offsetZ;
auto headPos = FCSE::GetTargetPoint(reference->As<RE::Actor>());
if (headPos) {
    offsetX = headPos->world.translate.x - reference->GetPosition().x;
    offsetY = headPos->world.translate.y - reference->GetPosition().y + 20.f;
    offsetZ = headPos->world.translate.z - reference->GetPosition().z;
}
log::info("Calculated offsets: X = {}, Y = {}, Z = {}", offsetX, offsetY, offsetZ);
                        timelineManager.AddTranslationPointAtCamera(0.0f, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtCamera(0.f, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtRef(0.5f, reference, 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtRef(1.5f, reference, 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddTranslationPointAtRef(2.f, reference, offsetX, offsetY, offsetZ, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtRef(2.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddTranslationPointAtRef(8.f, reference, offsetX, offsetY, offsetZ, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtRef(8.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtRef(9.f, RE::PlayerCharacter::GetSingleton(), 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddTranslationPointAtCamera(10.0f, true, true, InterpolationMode::kCubicHermite);
                        timelineManager.AddRotationPointAtCamera(10.f, true, true, InterpolationMode::kCubicHermite);
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
