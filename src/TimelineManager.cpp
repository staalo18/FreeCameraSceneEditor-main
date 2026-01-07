#include "TimelineManager.h"
#include "APIManager.h"

namespace FCSE {

    void TimelineManager::Initialize() {
        if (!APIs::FCFW) {
            return;
        }

        if (!APIs::FCFW->RegisterPlugin(SKSE::GetPluginHandle())) {
            log::error("TTE - {}: Could not register TTE plugin with FCFW!", __func__);
        }

        if (m_currentTimelineID == 0) {
            m_currentTimelineID = APIs::FCFW->RegisterTimeline(SKSE::GetPluginHandle());
log::info("{}: Registered timeline with ID {}", __FUNCTION__, m_currentTimelineID);
        }
    }

    void TimelineManager::Update() {
        if (!APIs::FCFW) {
            return;
        }

        if (m_currentTimelineID == 0) {
            return;
        }
    
        DrawTimeline();
    }

    size_t TimelineManager::GetTimelineID() {
        return m_currentTimelineID;
    }

    size_t TimelineManager::RegisterTimeline() {
        if (!APIs::FCFW) {
            return 0;
        }
        m_currentTimelineID = APIs::FCFW->RegisterTimeline(SKSE::GetPluginHandle());

        return m_currentTimelineID;
    }

    bool TimelineManager::UnregisterTimeline() {
        if (!APIs::FCFW) {
            return false;
        }

        bool result = APIs::FCFW->UnregisterTimeline(SKSE::GetPluginHandle(), m_currentTimelineID);

        if (result){
            CycleDown();
        }
        
        return result;
    }

    size_t TimelineManager::CycleUp() {
        if (!APIs::FCFW) {
            return 0;
        }

        m_currentTimelineID += 1;
        return m_currentTimelineID;
    } 

    size_t TimelineManager::CycleDown() {
        if (!APIs::FCFW) {
            return 0;
        }

        m_currentTimelineID -= 1;
        if (m_currentTimelineID < 0) {
            m_currentTimelineID = 0;
        }
        return m_currentTimelineID;
    }

    void TimelineManager::DrawTimeline() {
        if (!APIs::FCFW) {
            return;
        }

        if (m_currentTimelineID == 0 || !APIs::TrueHUD) {
            return;
        }

        auto handle = SKSE::GetPluginHandle();
        int translationCount = APIs::FCFW->GetTranslationPointCount(handle, m_currentTimelineID);
        int rotationCount = APIs::FCFW->GetRotationPointCount(handle, m_currentTimelineID);
        if (translationCount == 0 && rotationCount == 0) {
            return;
        }
        
        if (APIs::FCFW->IsPlaybackRunning(handle, m_currentTimelineID) || APIs::FCFW->IsRecording(handle, m_currentTimelineID)) {
            return;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        
        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }

// TEMP FIX to ensure TrueHUD menu is visible during timeline drawing
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto trueHUDMenu = ui->GetMenu<RE::IMenu>("TrueHUD");
            if (trueHUDMenu && trueHUDMenu->uiMovie) {
                trueHUDMenu->uiMovie->SetVisible(true);
            }
        }        
        
        // Draw lines between translation points
        for (size_t i = 0; i < translationCount - 1; ++i) {
            RE::NiPoint3 point1 = APIs::FCFW->GetTranslationPoint(handle, m_currentTimelineID, i);
            RE::NiPoint3 point2 = APIs::FCFW->GetTranslationPoint(handle, m_currentTimelineID, i + 1);
            APIs::TrueHUD->DrawLine(point1, point2);
        }
    }
} // namespace FCSE
