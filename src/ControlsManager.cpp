#include "ControlsManager.h"
#include "CameraPathManager.h"

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
                
                std::filesystem::path iniPath = std::filesystem::current_path() / "Data" / "SKSE" / "Plugins" / "FCSE_CameraPath.ini";

                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 4) {
                    // Key 4: Add translation point (no rotation)
float absTime = FCSE::CameraPathManager::GetSingleton().GetTranslationPointCount();
                    FCSE::CameraPathManager::GetSingleton().AddTranslationPoint( absTime, true, true);
                } else if (key == 5) {
                    // Key 5: Add rotation point (no translation)
float absTime = FCSE::CameraPathManager::GetSingleton().GetRotationPointCount();
                    FCSE::CameraPathManager::GetSingleton().AddRotationPoint( absTime, true, true);
                } else if (key == 6) {
                    FCSE::CameraPathManager::GetSingleton().ClearPath();
                } else if (key == 7) {
                    FCSE::CameraPathManager::GetSingleton().StartTraversal();
                } else if (key == 8) {
                    FCSE::CameraPathManager::GetSingleton().StartRecording();
                } else if (key == 9) {
                    FCSE::CameraPathManager::GetSingleton().StopRecording();
                } else if (key == 10) {
                    RE::DebugNotification("Exporting camera path...");
                    FCSE::CameraPathManager::GetSingleton().ExportCameraPath(iniPath.string().c_str());
                } else if (key == 11) {
                    RE::DebugNotification("Importing camera path...");
                    FCSE::CameraPathManager::GetSingleton().ImportCameraPath(iniPath.string().c_str());
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
