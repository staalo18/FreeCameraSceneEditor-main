#include "ControlsManager.h"
#include "TimelineManager.h"

namespace FCSE {

    float time = 0.0f;
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
                
                std::filesystem::path iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "FCSE_CameraPath.ini";

                const uint32_t key = buttonEvent->GetIDCode();
                 if (key == 5) {
                    RE::TESObjectREFR* reference = nullptr;
                    auto* form = RE::TESForm::LookupByID(0xd8c56);
                    reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                    reference = RE::TESForm::LookupByEditorID<RE::TESObjectREFR>("FCSE_TEST01");
                    if (reference) {
                        float offsetX = 0.0f;
                        float offsetY = 1000.0f;
                        float offsetZ = 0.0f;
                        bool isOffsetRelative = true;
                        FCSE::TimelineManager::GetSingleton().AddTranslationPointAtRef(time, reference, offsetX, offsetY, offsetZ, isOffsetRelative, false, false, 2);
                        FCSE::TimelineManager::GetSingleton().AddRotationPointAtRef(time, reference, PI, 0.f, false, false, 2);
                        time += 1.0f;
                    }
                } else if (key == 6) {
                    FCSE::TimelineManager::GetSingleton().ClearTimeline();
                } else if (key == 7) {
                    FCSE::TimelineManager::GetSingleton().StartTraversal(1.0f, false, false, false, 12.0f);
                } else if (key == 8) {
                    FCSE::TimelineManager::GetSingleton().StartRecording();
                } else if (key == 9) {
                    FCSE::TimelineManager::GetSingleton().StopRecording();
                } else if (key == 10) {
                    RE::DebugNotification("Exporting camera path...");
                    FCSE::TimelineManager::GetSingleton().ExportTimeline(iniPath.string().c_str());
                } else if (key == 11) {
                    RE::DebugNotification("Importing camera path...");
                    FCSE::TimelineManager::GetSingleton().ImportTimeline(iniPath.string().c_str());
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
