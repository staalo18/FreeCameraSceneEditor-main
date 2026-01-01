#include "ControlsManager.h"
#include "TimelineManager.h"

namespace FCSE {

size_t timelineID = 0;

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

                // Register timeline once for all operations
                auto& tm = FCSE::TimelineManager::GetSingleton();
                SKSE::PluginHandle handle = SKSE::GetPluginHandle();
                if (timelineID == 0) {
                    timelineID = tm.RegisterTimeline(handle);
                }

                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 2) {
                    if (tm.IsPlaybackPaused(timelineID)) {
                        tm.ResumePlayback(timelineID);
                    } else {
                        tm.PausePlayback(timelineID);
                    }
                } else if (key == 3) {
                    tm.StopPlayback(timelineID);
                } else if (key == 4) {
                    // Toggle user rotation for current timeline
                    size_t activeID = tm.GetActiveTimelineID();
                    if (activeID != 0) {
                        tm.AllowUserRotation(activeID, !tm.IsUserRotationAllowed(activeID));
                    }
                } else if (key == 5) {
                    RE::TESObjectREFR* reference = nullptr;
                    auto* form = RE::TESForm::LookupByID(0xd8c56);
                    reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                    if (reference) {
                        bool isOffsetRelative = true;
                        
                        // Calculate offsets
                        float offsetX = 0, offsetY = 0, offsetZ = 0;
                        auto headPos = FCSE::GetTargetPoint(reference->As<RE::Actor>());
                        if (headPos) {
                            offsetX = headPos->world.translate.x - reference->GetPosition().x;
                            offsetY = headPos->world.translate.y - reference->GetPosition().y + 20.f;
                            offsetZ = headPos->world.translate.z - reference->GetPosition().z;
                        }
                        log::info("Calculated offsets: X = {}, Y = {}, Z = {}", offsetX, offsetY, offsetZ);
                        
                        // Build timeline with exact same structure as original
                        tm.AddTranslationPointAtCamera(timelineID, handle, 0.0f, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtCamera(timelineID, handle, 0.f, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtRef(timelineID, handle, 0.5f, reference, 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtRef(timelineID, handle, 1.5f, reference, 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        tm.AddTranslationPointAtRef(timelineID, handle, 2.f, reference, offsetX, offsetY, offsetZ, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtRef(timelineID, handle, 2.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        tm.AddTranslationPointAtRef(timelineID, handle, 8.f, reference, offsetX, offsetY, offsetZ, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtRef(timelineID, handle, 8.f, reference, 0.0f, 0.f, isOffsetRelative, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtRef(timelineID, handle, 9.f, RE::PlayerCharacter::GetSingleton(), 0.0f, 0.f, false, true, true, InterpolationMode::kCubicHermite);
                        tm.AddTranslationPointAtCamera(timelineID, handle, 10.0f, true, true, InterpolationMode::kCubicHermite);
                        tm.AddRotationPointAtCamera(timelineID, handle, 10.f, true, true, InterpolationMode::kCubicHermite);
                        
                        log::info("Created timeline {} with reference tracking", timelineID);
                        tm.StartPlayback(timelineID, 1.0f, false, false, false, 0.0f);
                    }
                } else if (key == 6) {
                    tm.ClearTimeline(timelineID, handle, false);
                } else if (key == 7) {
                    tm.StartPlayback(timelineID, 1.0f, false, false, false, 0.0f);
                } else if (key == 8) {
                    tm.StartRecording(timelineID, handle);
                } else if (key == 9) {
                    tm.StopRecording(timelineID, handle);
                } else if (key == 10) {
                    RE::DebugNotification("Exporting camera path...");
                    tm.ExportTimeline(timelineID, relativePath);
                } else if (key == 11) {
                    RE::DebugNotification("Importing camera path...");
                    tm.AddTimelineFromFile(timelineID, handle, relativePath);
                } else if (key == 20) { // T
                    timelineID = tm.RegisterTimeline(handle);
                } else if (key == 21) { // Y
                    tm.UnregisterTimeline(timelineID, handle);
                    timelineID -= 1;
                    if (timelineID < 0) { timelineID = 0;}
                } else if (key == 22) { // U
                    timelineID += 1;
                } else if (key == 35) { // H
                    if (timelineID > 1) {
                        timelineID -= 1;
                    }
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
