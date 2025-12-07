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
                
                const uint32_t key = buttonEvent->GetIDCode();
                if (key == 4) {
                    // Key 4: Add translation point (no rotation)
                    FCSE::CameraPathManager::GetSingleton().AddPathPoint(FCSE::TimelineType::Translation);
                } else if (key == 5) {
                    // Key 5: Add rotation point (no translation)
                    FCSE::CameraPathManager::GetSingleton().AddPathPoint(FCSE::TimelineType::Rotation);
                } else if (key == 6) {
                    FCSE::CameraPathManager::GetSingleton().ClearPath();
                } else if (key == 7) {
                    FCSE::CameraPathManager::GetSingleton().StartTraversal();
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
