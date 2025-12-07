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
                    FCSE::CameraPathManager::GetSingleton().AddPoint();
                    // Future use
                } else if (key == 5) {
                    FCSE::CameraPathManager::GetSingleton().StartTraversal();
                    // Future use
                }
            }
        }

        return RE::BSEventNotifyControl::kContinue;
    }
} // namespace FCSE
