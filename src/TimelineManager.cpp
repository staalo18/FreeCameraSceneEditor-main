#include "TimelineManager.h"
#include "APIManager.h"

namespace FCSE {

    void TimelineManager::Update() {
        if (RE::UI::GetSingleton()->GameIsPaused()) {
            return;
        }

        DrawTimeline();

        TraverseCamera();

        RecordTimeline();
    }

    void TimelineManager::StartRecording() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (m_isRecording || !(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }

        RE::DebugNotification("Starting camera path recording...");

        m_isRecording = true;
        m_currentRecordingTime = 0.0f;
        m_lastRecordedPointTime = 0.0f;

        ClearTimeline(false);

        AddTranslationPoint(m_currentRecordingTime, true, false);
        AddRotationPoint(m_currentRecordingTime, true, false);
    }

    void TimelineManager::StopRecording() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (!m_isRecording || !(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }
        
        AddTranslationPoint(m_currentRecordingTime, false, true);
        AddRotationPoint(m_currentRecordingTime, false, true);

        RE::DebugNotification("Camera path recording stopped.");

        m_isRecording = false;
    }

    void TimelineManager::RecordTimeline() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            m_currentRecordingTime += _ts_SKSEFunctions::GetRealTimeDeltaTime();

            if (m_isRecording) {
                if (m_currentRecordingTime - m_lastRecordedPointTime >= m_recordingInterval) {
                    AddTranslationPoint(m_currentRecordingTime, false, false);
                    AddRotationPoint(m_currentRecordingTime, false, false);
                    
                    m_lastRecordedPointTime = m_currentRecordingTime;
                }
            }
        } else {
            if (m_isRecording) {
                StopRecording();
            }
        }
    }

    size_t TimelineManager::AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut) {
        return AddTranslationPoint(m_translationTimeline.GetPointAtCameraPos(a_time, a_easeIn, a_easeOut));
    }

    size_t TimelineManager::AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut) {
        return AddRotationPoint(m_rotationTimeline.GetPointAtCameraPos(a_time, a_easeIn, a_easeOut));
    }

    size_t TimelineManager::AddTranslationPoint(const TranslationPoint& a_point) {
        return m_translationTimeline.AddPoint(a_point);
    }

    size_t TimelineManager::AddRotationPoint(const RotationPoint& a_point) {
        return m_rotationTimeline.AddPoint(a_point);
    }

    const TranslationPoint& TimelineManager::GetTranslationPoint(size_t a_index) const {
        return m_translationTimeline.GetPoint(a_index);
    }

    const RotationPoint& TimelineManager::GetRotationPoint(size_t a_index) const {
        return m_rotationTimeline.GetPoint(a_index);
    }

    size_t TimelineManager::UpdateTranslationPoint(size_t a_index, const TranslationPoint& a_point) {
        return m_translationTimeline.UpdatePoint(a_index, a_point);
    }

    size_t TimelineManager::UpdateRotationPoint(size_t a_index, const RotationPoint& a_point) {
        return m_rotationTimeline.UpdatePoint(a_index, a_point);
    }

    void TimelineManager::RemoveTranslationPoint(size_t a_index) {
        m_translationTimeline.RemovePoint(a_index);
    }

    void TimelineManager::RemoveRotationPoint(size_t a_index) {
        m_rotationTimeline.RemovePoint(a_index);
    }

    void TimelineManager::TraverseCamera() {
        if (!m_isTraversing || m_isRecording) {
            return;
        }

        if (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) {
            StopTraversal();
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            log::error("{}: PlayerCamera not found during traversal", __FUNCTION__);
            StopTraversal();
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            StopTraversal();
            return;
        }

        m_currentTraversalTime += _ts_SKSEFunctions::GetRealTimeDeltaTime();

        m_translationTimeline.UpdateTimeline(m_currentTraversalTime);
        m_rotationTimeline.UpdateTimeline(m_currentTraversalTime);
        
        // Get interpolated points from timeline
        RE::NiPoint3 currentPosition = m_translationTimeline.GetCurrentPoint();
        RE::BSTPoint2<float> currentRotation = m_rotationTimeline.GetCurrentPoint();

        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        if (!cameraState) {
            StopTraversal();
            return;
        }

        cameraState->translation = currentPosition;
        cameraState->rotation = currentRotation;
        
        // Check if both timelines are complete
        if (m_translationTimeline.IsComplete() && m_rotationTimeline.IsComplete()) {
            StopTraversal();
        }        
    }

    void TimelineManager::DrawTimeline() {
        if (!APIs::TrueHUD || (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) || m_isTraversing) {
            return;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }

        if (m_isRecording) {
            return;
        }

        SetHUDMenuVisible(true);
        
        // Draw translation path (lines between translation points)
        for (size_t i = 0; i + 1 < m_translationTimeline.GetPointCount(); ++i) {
            const auto& point1 = m_translationTimeline.GetPoint(i);
            const auto& point2 = m_translationTimeline.GetPoint(i + 1);
            if (point1.m_transition.m_interpolationType != InterpolationType::kInvalid &&
                point2.m_transition.m_interpolationType != InterpolationType::kInvalid) {
                APIs::TrueHUD->DrawLine(point1.m_point, point2.m_point);
            }
        }
    }


    void TimelineManager::ClearTimeline(bool a_notify) {
        if (m_isRecording) {
            return;
        }

        if (a_notify) {
            RE::DebugNotification("Clearing camera path...");
        }

        m_translationTimeline.ClearTimeline();
        m_rotationTimeline.ClearTimeline();

        StopTraversal();
    }

    void TimelineManager::StartTraversal() {
        if (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) {
            log::info("{}: Need at least 1 point to traverse", __FUNCTION__);
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (playerCamera->IsInFreeCameraMode()) {
            return;
        }

        if (m_isRecording || m_isTraversing) {
            return;
        }
        
        m_isTraversing = true;
        m_currentTraversalTime = 0.0f;
        
        m_translationTimeline.ResetTimeline();
        m_rotationTimeline.ResetTimeline();        

        playerCamera->ToggleFreeCameraMode(false);
    }

    void TimelineManager::StopTraversal() {

        if (m_isTraversing) {
            auto* playerCamera = RE::PlayerCamera::GetSingleton();
            if (playerCamera && playerCamera->IsInFreeCameraMode()) {
                playerCamera->ToggleFreeCameraMode(false);
            }            
        }

        m_isTraversing = false;
        m_currentTraversalTime = 0.0f;
        
        m_translationTimeline.ResetTimeline();
        m_rotationTimeline.ResetTimeline();        
    }

    bool TimelineManager::ImportTimeline(const char* a_filePath) {
        std::filesystem::path fullPath(a_filePath);
        std::string filename = fullPath.filename().string();
        
        if (!std::filesystem::exists(fullPath)) {
            log::error("{}: File does not exist: {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        ClearTimeline(false);
        
        // Read General section
        bool useDegrees = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "UseDegrees:General", filename, true);
        float degToRad = useDegrees ? (PI / 180.0f) : 1.0f;
        
log::info("{}: Importing timeline from {}, UseDegrees={}", __FUNCTION__, a_filePath, useDegrees);
        
        std::ifstream file(a_filePath);
        if (!file.is_open()) {
            log::error("{}: Failed to open file for reading: {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        bool importTranslationSuccess = m_translationTimeline.ImportTimeline(file, 1.0f);
        
        // Rewind file to beginning for rotation import
        file.clear();  // Clear any error flags
        file.seekg(0, std::ios::beg);
        if (!file.good()) {
            log::error("{}: Failed to rewind file: {}", __FUNCTION__, a_filePath);
            file.close();
            return false;
        }
        
        bool importRotationSuccess = m_rotationTimeline.ImportTimeline(file, degToRad);
        
        file.close();
        
        if (!importTranslationSuccess) {
            log::error("{}: Failed to import translation points from {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        if (!importRotationSuccess) {
            log::error("{}: Failed to import rotation points from {}", __FUNCTION__, a_filePath);
            return false;
        }

log::info("{}: Loaded {} translation and {} rotation points from {}", 
__FUNCTION__, m_translationTimeline.GetPointCount(), m_rotationTimeline.GetPointCount(), a_filePath);
        return true;
    }
    
    bool TimelineManager::ExportTimeline(const char* a_filePath) const {
        std::ofstream file(a_filePath);
        if (!file.is_open()) {
            log::error("{}: Failed to open file for writing: {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        // Write General section
        file << "[General]\n";
        file << "UseDegrees=1 ; Interpret rotation values as degrees (1) or radians (0)\n";
        file << "\n";
        
        float radToDeg = 180.0f / PI;

        bool exportTranslationSuccess = m_translationTimeline.ExportTimeline(file, 1.0f);
        bool exportRotationSuccess = m_rotationTimeline.ExportTimeline(file, radToDeg);
                
        file.close();

        if (!exportTranslationSuccess || !exportRotationSuccess) {
            log::error("{}: Failed to export points to {}", __FUNCTION__, a_filePath);
            return false;
        }
                
        if (!file.good()) {
            log::error("{}: Error occurred while writing file: {}", __FUNCTION__, a_filePath);
            return false;
        }
        
log::info("{}: Exported {} translation and {} rotation points to {}", 
__FUNCTION__, m_translationTimeline.GetPointCount(), m_rotationTimeline.GetPointCount(), a_filePath);
        return true;
    }
} // namespace FCSE