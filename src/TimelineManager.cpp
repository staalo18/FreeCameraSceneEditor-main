#include "TimelineManager.h"
#include "APIManager.h"

namespace FCSE {

    void TimelineManager::Update() {
        auto* ui = RE::UI::GetSingleton();
        if (ui && ui->GameIsPaused()) {

            if (m_isPlaybackRunning) {
                // re-enable menus if paused during playback
                ui->ShowMenus(m_isShowingMenus);
            }
            return;
        } else if (m_isPlaybackRunning) {
            ui->ShowMenus(m_showMenusDuringPlayback);
        } 

        DrawTimeline();

        PlayTimeline();

        RecordTimeline();
    }

    void TimelineManager::StartRecording() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (m_isRecording || m_isPlaybackRunning || !(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }

        RE::DebugNotification("Starting camera path recording...");

        m_isRecording = true;
        m_currentRecordingTime = 0.0f;
        m_lastRecordedPointTime = 0.0f;

        ClearTimeline(false);

        AddTranslationPointAtCamera(m_currentRecordingTime, true, false, 2);
        AddRotationPointAtCamera(m_currentRecordingTime, true, false, 2);
    }

    void TimelineManager::StopRecording() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (!m_isRecording || !(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }
        
        AddTranslationPointAtCamera(m_currentRecordingTime, false, true, 2);
        AddRotationPointAtCamera(m_currentRecordingTime, false, true, 2);

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
                    AddTranslationPointAtCamera(m_currentRecordingTime, false, false, 2);
                    AddRotationPointAtCamera(m_currentRecordingTime, false, false, 2);
                    
                    m_lastRecordedPointTime = m_currentRecordingTime;
                }
            }
        } else {
            if (m_isRecording) {
                StopRecording();
            }
        }
    }

    size_t TimelineManager::AddTranslationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        auto point = m_translationTimeline.GetPointAtCamera(a_time, a_easeIn, a_easeOut);
        point.m_transition.m_mode = ToInterpolationMode(a_interpolationMode);
        return AddTranslationPoint(point);
    }

    size_t TimelineManager::AddTranslationPoint(float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::NiPoint3 position(a_posX, a_posY, a_posZ);
        TranslationPoint point(transition, position);
        return AddTranslationPoint(point);
    }

    size_t TimelineManager::AddTranslationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        if (!a_reference) {
            log::warn("{}: Null reference provided, creating point at origin", __FUNCTION__);
            return AddTranslationPoint(a_time, a_offsetX, a_offsetY, a_offsetZ, a_easeIn, a_easeOut, a_interpolationMode);
        }
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::NiPoint3 offset(a_offsetX, a_offsetY, a_offsetZ);
        TranslationPoint point(transition, a_reference, offset, a_isOffsetRelative);
        return AddTranslationPoint(point);
    }

    size_t TimelineManager::AddRotationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        auto point = m_rotationTimeline.GetPointAtCamera(a_time, a_easeIn, a_easeOut);
        point.m_transition.m_mode = ToInterpolationMode(a_interpolationMode);
        return AddRotationPoint(point);
    }

    size_t TimelineManager::AddRotationPoint(float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::BSTPoint2<float> rotation({a_pitch, a_yaw});
        RotationPoint point(transition, rotation);
        return AddRotationPoint(point);
    }

    size_t TimelineManager::AddRotationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        if (!a_reference) {
            log::warn("{}: Null reference provided, creating point with offset as absolute rotation", __FUNCTION__);
            return AddRotationPoint(a_time, a_offsetPitch, a_offsetYaw, a_easeIn, a_easeOut, a_interpolationMode);
        }
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::BSTPoint2<float> offset({a_offsetPitch, a_offsetYaw});
        RotationPoint point(transition, a_reference, offset, a_isOffsetRelative);
        return AddRotationPoint(point);
    }    size_t TimelineManager::AddTranslationPoint(const TranslationPoint& a_point) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        return m_translationTimeline.AddPoint(a_point);
    }

    size_t TimelineManager::AddRotationPoint(const RotationPoint& a_point) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        return m_rotationTimeline.AddPoint(a_point);
    }

    const TranslationPoint& TimelineManager::GetTranslationPoint(size_t a_index) const {
        return m_translationTimeline.GetPoint(a_index);
    }

    const RotationPoint& TimelineManager::GetRotationPoint(size_t a_index) const {
        return m_rotationTimeline.GetPoint(a_index);
    }

    size_t TimelineManager::EditTranslationPoint(size_t a_index, const TranslationPoint& a_point) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        return m_translationTimeline.EditPoint(a_index, a_point);
    }

    size_t TimelineManager::EditRotationPoint(size_t a_index, const RotationPoint& a_point) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        return m_rotationTimeline.EditPoint(a_index, a_point);
    }

    size_t TimelineManager::EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::NiPoint3 position(a_posX, a_posY, a_posZ);
        TranslationPoint point(transition, position);
        return EditTranslationPoint(a_index, point);
    }

    size_t TimelineManager::EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, int a_interpolationMode) {
        Transition transition(a_time, ToInterpolationMode(a_interpolationMode), a_easeIn, a_easeOut);
        RE::BSTPoint2<float> rotation({a_pitch, a_yaw});
        RotationPoint point(transition, rotation);
        return EditRotationPoint(a_index, point);
    }

    void TimelineManager::RemoveTranslationPoint(size_t a_index) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        m_translationTimeline.RemovePoint(a_index);
    }

    void TimelineManager::RemoveRotationPoint(size_t a_index) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }
        m_rotationTimeline.RemovePoint(a_index);
    }

    void TimelineManager::PlayTimeline() {
        if (!m_isPlaybackRunning || m_isRecording) {
            return;
        }

        if (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) {
            StopPlayback();
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            log::error("{}: PlayerCamera not found during playback", __FUNCTION__);
            StopPlayback();
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            StopPlayback();
            return;
        }
        
        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        if (!cameraState) {
            log::error("{}: FreeCameraState not found during playback", __FUNCTION__);
            StopPlayback();
            return;
        }

        float deltaTime = _ts_SKSEFunctions::GetRealTimeDeltaTime() * m_playbackSpeed;
        
        m_translationTimeline.UpdateTimeline(deltaTime);
        m_rotationTimeline.UpdateTimeline(deltaTime);

        // Apply global easing to determine which point in the timeline to sample
        float sampleTime = m_translationTimeline.GetPlaybackTime();
        if (m_globalEaseIn || m_globalEaseOut) {
            float timelineDuration = GetTimelineDuration();
            
            if (timelineDuration > 0.0f) {
                float linearProgress = std::clamp(sampleTime / timelineDuration, 0.0f, 1.0f);
                float easedProgress = _ts_SKSEFunctions::ApplyEasing(linearProgress, m_globalEaseIn, m_globalEaseOut);
                
                sampleTime = easedProgress * timelineDuration;
            }
        }

        // Get interpolated points from timeline at the (potentially eased) sample time
        cameraState->translation = m_translationTimeline.GetPointAtTime(sampleTime);
        RE::BSTPoint2<float> rotation = m_rotationTimeline.GetPointAtTime(sampleTime);

        if (m_userTurning && m_allowUserRotation) {
            // User is actively controlling rotation - update offset based on difference between
            // current camera rotation and timeline rotation
            m_rotationOffset.x = _ts_SKSEFunctions::NormalRelativeAngle(cameraState->rotation.x - rotation.x);
            m_rotationOffset.y = _ts_SKSEFunctions::NormalRelativeAngle(cameraState->rotation.y - rotation.y);
            // reset m_userTurning - Is set to true in LookHook::ProcessMouseMove() in case of user-triggered camera rotation
            m_userTurning = false;
            // Don't update cameraState->rotation, user is controlling it
        } else {
            // User not controlling rotation - apply timeline rotation plus accumulated offset
            cameraState->rotation.x = _ts_SKSEFunctions::NormalRelativeAngle(rotation.x + m_rotationOffset.x);
            cameraState->rotation.y = _ts_SKSEFunctions::NormalRelativeAngle(rotation.y + m_rotationOffset.y);
        }

        // Check if both timelines have completed (for kEnd mode)
        if (!m_translationTimeline.IsPlaying() && !m_rotationTimeline.IsPlaying()) {
            StopPlayback();
        }
    }

    float TimelineManager::GetTimelineDuration() const {
        float duration = 0.0f;
        if (m_translationTimeline.GetPointCount() > 0) {
            duration = std::max(duration, 
                m_translationTimeline.GetPoint(m_translationTimeline.GetPointCount() - 1).m_transition.m_time);
        }
        if (m_rotationTimeline.GetPointCount() > 0) {
            duration = std::max(duration,
                m_rotationTimeline.GetPoint(m_rotationTimeline.GetPointCount() - 1).m_transition.m_time);
        }
        return duration;
    }

    void TimelineManager::DrawTimeline() {
        if (!APIs::TrueHUD || (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) || m_isPlaybackRunning) {
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
            APIs::TrueHUD->DrawLine(point1.m_point, point2.m_point);
        }
    }


    void TimelineManager::ClearTimeline(bool a_notifyUser) {
        if (m_isRecording) {
            return;
        }

        if (a_notifyUser) {
            RE::DebugNotification("Clearing camera path...");
        }

        m_translationTimeline.ClearTimeline();
        m_rotationTimeline.ClearTimeline();

        StopPlayback();
    }

    void TimelineManager::StartPlayback(float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) {

        if (m_translationTimeline.GetPointCount() == 0 && m_rotationTimeline.GetPointCount() == 0) {
            log::info("{}: Need at least 1 point to play timeline", __FUNCTION__);
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (playerCamera->IsInFreeCameraMode()) {
            return;
        }

        if (m_isRecording || m_isPlaybackRunning) {
            return;
        }
        
        float timelineDuration = GetTimelineDuration();
        if (timelineDuration <= 0.0f && !a_useDuration) {
            log::info("{}: Timeline duration is zero, cannot play", __FUNCTION__);
            return;
        }

        if (a_useDuration) {
            if (a_duration <= 0.0f) {
                log::warn("{}: Invalid duration {}, defaulting to timeline duration", __FUNCTION__, a_duration);
                m_playbackDuration = timelineDuration;            
                m_playbackSpeed = 1.0f;
            } else {
                m_playbackDuration = a_duration;
                m_playbackSpeed = timelineDuration / m_playbackDuration;
            }
        } else {
            if (a_speed <= 0.0f) {
                log::warn("{}: Invalid speed {}, defaulting to 1.0", __FUNCTION__, a_speed);
                m_playbackDuration = timelineDuration;            
                m_playbackSpeed = 1.0f;
            } else {
                m_playbackDuration = timelineDuration / a_speed;
                m_playbackSpeed = a_speed;
            }
        }

        if (m_playbackDuration <= 0.0f) {
            log::info("{}: Playback duration is zero, cannot play", __FUNCTION__);
            return;
        }
        
        m_globalEaseIn = a_globalEaseIn;
        m_globalEaseOut = a_globalEaseOut;
        
        m_isPlaybackRunning = true;
        m_rotationOffset = { 0.0f, 0.0f };
        
        if (playerCamera->currentState && 
            (playerCamera->currentState->id == RE::CameraState::kThirdPerson ||
			playerCamera->currentState->id == RE::CameraState::kMount ||
			playerCamera->currentState->id == RE::CameraState::kDragon)) {
				
            RE::ThirdPersonState* cameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
				
            if (cameraState) {
                m_lastFreeRotation = cameraState->freeRotation;
            }
        }
        
        m_translationTimeline.ResetTimeline();
        m_rotationTimeline.ResetTimeline();
        m_translationTimeline.StartPlayback();
        m_rotationTimeline.StartPlayback();
        
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            m_isShowingMenus = ui->IsShowingMenus();
            ui->ShowMenus(m_showMenusDuringPlayback);
        }
        playerCamera->ToggleFreeCameraMode(false);
    }

    void TimelineManager::StopPlayback() {

        if (m_isPlaybackRunning) {
            auto* playerCamera = RE::PlayerCamera::GetSingleton();
            if (playerCamera && playerCamera->IsInFreeCameraMode()) {
                auto* ui = RE::UI::GetSingleton();
                if (ui) {
                    ui->ShowMenus(m_isShowingMenus);
                }     
                playerCamera->ToggleFreeCameraMode(false);

                if (playerCamera->currentState && 
                    (playerCamera->currentState->id == RE::CameraState::kThirdPerson ||
                    playerCamera->currentState->id == RE::CameraState::kMount ||
                    playerCamera->currentState->id == RE::CameraState::kDragon)) {
                        
                    RE::ThirdPersonState* cameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
                        
                    if (cameraState) {
                        cameraState->freeRotation = m_lastFreeRotation;
                    }
                }
            }            
        }

        m_isPlaybackRunning = false;
        m_playbackSpeed = 1.0f;
        m_globalEaseIn = false;
        m_globalEaseOut = false;
        
        m_translationTimeline.StopPlayback();
        m_rotationTimeline.StopPlayback();
        m_translationTimeline.ResetTimeline();
        m_rotationTimeline.ResetTimeline();
    }

    void TimelineManager::SetUserTurning(bool a_turning) {
        m_userTurning = a_turning;
    }

    void TimelineManager::PausePlayback() {
        if (m_isPlaybackRunning) {
            m_translationTimeline.PausePlayback();
            m_rotationTimeline.PausePlayback();
        }
    }

    void TimelineManager::ResumePlayback() {
        if (m_isPlaybackRunning) {
            m_translationTimeline.ResumePlayback();
            m_rotationTimeline.ResumePlayback();
        }
    }

    bool TimelineManager::IsPlaybackPaused() const {
        return m_isPlaybackRunning && (m_translationTimeline.IsPaused() || m_rotationTimeline.IsPaused());
    }

    bool TimelineManager::AddTimelineFromFile(const char* a_filePath, float a_timeOffset) {
        if (m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback();
        }

        std::filesystem::path fullPath = std::filesystem::current_path() / "Data" / a_filePath;
        
        if (!std::filesystem::exists(fullPath)) {
            log::error("{}: File does not exist: {}", __FUNCTION__, fullPath.string());
            return false;
        }
        
        // Read General section
        long fileVersion = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "Version:General", a_filePath, 0L);
        bool useDegrees = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "UseDegrees:General", a_filePath, true);
        long playbackModeValue = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "PlaybackMode:General", a_filePath, 0L);  // Default to kEnd
        float loopTimeOffset = static_cast<float>(_ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "LoopTimeOffset:General", a_filePath, 0.0));  // Default to 0.0
        
        float degToRad = useDegrees ? (PI / 180.0f) : 1.0f;
        PlaybackMode playbackMode = (playbackModeValue == 1) ? PlaybackMode::kLoop : PlaybackMode::kEnd;
        
        // Calculate current plugin version
        long pluginVersion = static_cast<long>(Plugin::VERSION[0]) * 10000 + 
                            static_cast<long>(Plugin::VERSION[1]) * 100 + 
                            static_cast<long>(Plugin::VERSION[2]);
        
        if (fileVersion != pluginVersion) {
            log::info("{}: Importing timeline from {} - File version {} differs from plugin version {}", 
                     __FUNCTION__, a_filePath, fileVersion, pluginVersion);
        }
              
        std::ifstream file(fullPath);
        if (!file.is_open()) {
            log::error("{}: Failed to open file for reading: {}", __FUNCTION__, fullPath.string());
            return false;
        }
        
        size_t translationPointCount = m_translationTimeline.GetPointCount();
        size_t rotationPointCount = m_rotationTimeline.GetPointCount();

        bool importTranslationSuccess = m_translationTimeline.AddTimelineFromFile(file, a_timeOffset, 1.0f);
        
        // Rewind file to beginning for rotation import
        file.clear();  // Clear any error flags
        file.seekg(0, std::ios::beg);
        if (!file.good()) {
            log::error("{}: Failed to rewind file: {}", __FUNCTION__, fullPath.string());
            file.close();
            return false;
        }
        
        bool importRotationSuccess = m_rotationTimeline.AddTimelineFromFile(file, a_timeOffset, degToRad);
        
        // Set playback mode and offset on both timelines
        m_translationTimeline.SetPlaybackMode(playbackMode);
        m_translationTimeline.SetLoopTimeOffset(loopTimeOffset);
        m_rotationTimeline.SetPlaybackMode(playbackMode);
        m_rotationTimeline.SetLoopTimeOffset(loopTimeOffset);
        
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
__FUNCTION__, m_translationTimeline.GetPointCount() - translationPointCount, 
m_rotationTimeline.GetPointCount() - rotationPointCount, a_filePath);

        return true;
    }
    
    bool TimelineManager::ExportTimeline(const char* a_filePath) const {
        // Convert relative path to absolute path from Data folder
        std::filesystem::path fullPath = std::filesystem::current_path() / "Data" / a_filePath;
        
        std::ofstream file(fullPath);
        if (!file.is_open()) {
            log::error("{}: Failed to open file for writing: {}", __FUNCTION__, fullPath.string());
            return false;
        }
        
        // Write General section
        // Encode version as: major * 10000 + minor * 100 + patch
        int versionInt = static_cast<int>(Plugin::VERSION[0]) * 10000 + 
                         static_cast<int>(Plugin::VERSION[1]) * 100 + 
                         static_cast<int>(Plugin::VERSION[2]);
        file << "[General]\n";
        file << "Version=" << versionInt << "\n";
        file << "UseDegrees=1 ; Interpret rotation values as degrees (1) or radians (0)\n";
        
        // Get playback mode and offset from translation timeline (both timelines should have same values)
        int playbackModeInt = static_cast<int>(m_translationTimeline.GetPlaybackMode());
        float loopTimeOffset = m_translationTimeline.GetLoopTimeOffset();
        file << "PlaybackMode=" << playbackModeInt << " ; 0=kEnd (stop at end), 1=kLoop (restart from beginning)\n";
        file << "LoopTimeOffset=" << loopTimeOffset << " ; Extra time after last point for loop interpolation (seconds)\n";
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