#include "TimelineManager.h"
#include "APIManager.h"
#include "FCSE_Utils.h"

namespace FCSE {

    void TimelineManager::Update() {
        // Hold lock for entire Update() to prevent race conditions
        // This ensures the timeline cannot be deleted/modified while we're using it
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        // Check for active timeline
        if (m_activeTimelineID == 0) {
            return;
        }
        
        TimelineState* activeState = GetTimeline(m_activeTimelineID);
        if (!activeState) {
            return;
        }
        
        auto* ui = RE::UI::GetSingleton();
        
        // Handle game pause
        if (ui && ui->GameIsPaused()) {
            if (activeState->m_isPlaybackRunning) {
                ui->ShowMenus(m_isShowingMenus);
            }
            return;
        }
        
        // Update UI visibility during playback
        if (activeState->m_isPlaybackRunning) {
            ui->ShowMenus(activeState->m_showMenusDuringPlayback);
        }
        
        // Execute timeline operations under lock protection
        DrawTimeline(activeState);
        PlayTimeline(activeState);
        RecordTimeline(activeState);
    }

    bool TimelineManager::StartRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        // Check if any timeline is already active
        if (m_activeTimelineID != 0) {
            log::error("{}: Timeline {} is already active", __FUNCTION__, m_activeTimelineID);
            return false;
        }
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            log::error("{}: PlayerCamera not available", __FUNCTION__);
            return false;
        }
        
        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            log::error("{}: Must be in free camera mode", __FUNCTION__);
            return false;
        }
        
        // Set as active timeline
        m_activeTimelineID = a_timelineID;
        state->m_isRecording = true;
        state->m_currentRecordingTime = 0.0f;
        state->m_lastRecordedPointTime = -m_recordingInterval;  // Ensure first point is captured immediately
        
        // Clear existing points
        state->m_timeline.ClearPoints();
        
        // Add initial point
        RE::NiPoint3 cameraPos = _ts_SKSEFunctions::GetCameraPos();
        RE::NiPoint3 cameraRot = _ts_SKSEFunctions::GetCameraRotation();
        
        Transition transTranslation(0.0f, InterpolationMode::kCubicHermite, true, false);
        TranslationPoint translationPoint(transTranslation, PointType::kWorld, cameraPos);
        state->m_timeline.AddTranslationPoint(translationPoint);
        
        Transition transRotation(0.0f, InterpolationMode::kCubicHermite, true, false);
        RotationPoint rotationPoint(transRotation, PointType::kWorld, RE::BSTPoint2<float>({cameraRot.x, cameraRot.z}));
        state->m_timeline.AddRotationPoint(rotationPoint);
        
        RE::DebugNotification("Starting camera path recording...");
        log::info("{}: Started recording on timeline {}", __FUNCTION__, a_timelineID);
        return true;
    }

    bool TimelineManager::StopRecording(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        if (!state->m_isRecording) {
            log::warn("{}: Timeline {} is not recording", __FUNCTION__, a_timelineID);
            return false;
        }
        
        if (m_activeTimelineID != a_timelineID) {
            log::error("{}: Timeline {} is not the active timeline", __FUNCTION__, a_timelineID);
            return false;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return false;
        }
        
        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            log::warn("{}: Not in free camera mode", __FUNCTION__);
        }
        
        // Add final point
        RE::NiPoint3 cameraPos = _ts_SKSEFunctions::GetCameraPos();
        RE::NiPoint3 cameraRot = _ts_SKSEFunctions::GetCameraRotation();
        
        Transition transTranslation(state->m_currentRecordingTime, InterpolationMode::kCubicHermite, false, true);
        TranslationPoint translationPoint(transTranslation, PointType::kWorld, cameraPos);
        state->m_timeline.AddTranslationPoint(translationPoint);
        
        Transition transRotation(state->m_currentRecordingTime, InterpolationMode::kCubicHermite, false, true);
        RotationPoint rotationPoint(transRotation, PointType::kWorld, RE::BSTPoint2<float>({cameraRot.x, cameraRot.z}));
        state->m_timeline.AddRotationPoint(rotationPoint);
        
        // Clear recording state
        m_activeTimelineID = 0;
        state->m_isRecording = false;
        
        RE::DebugNotification("Camera path recording stopped.");
        log::info("{}: Stopped recording on timeline {}", __FUNCTION__, a_timelineID);
        return true;
    }

    int TimelineManager::AddTranslationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        TranslationPoint point = state->m_timeline.GetTranslationPointAtCamera(a_time, a_easeIn, a_easeOut);
        point.m_transition = transition;
        
        return static_cast<int>(state->m_timeline.AddTranslationPoint(point));
    }

    int TimelineManager::AddTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        TranslationPoint point(transition, PointType::kWorld, RE::NiPoint3(a_posX, a_posY, a_posZ));
        
        return static_cast<int>(state->m_timeline.AddTranslationPoint(point));
    }

    int TimelineManager::AddTranslationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (!a_reference) {
            log::error("{}: Null reference provided", __FUNCTION__);
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        RE::NiPoint3 offset(a_offsetX, a_offsetY, a_offsetZ);
        TranslationPoint point(transition, PointType::kReference, RE::NiPoint3{}, offset, a_reference, a_isOffsetRelative);
        
        return static_cast<int>(state->m_timeline.AddTranslationPoint(point));
    }

    int TimelineManager::AddRotationPointAtCamera(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        RotationPoint point = state->m_timeline.GetRotationPointAtCamera(a_time, a_easeIn, a_easeOut);
        point.m_transition = transition;
        
        return static_cast<int>(state->m_timeline.AddRotationPoint(point));
    }

    int TimelineManager::AddRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        RotationPoint point(transition, PointType::kWorld, RE::BSTPoint2<float>({a_pitch, a_yaw}));
        
        return static_cast<int>(state->m_timeline.AddRotationPoint(point));
    }

    int TimelineManager::AddRotationPointAtRef(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return -1;
        }
        
        if (!a_reference) {
            log::error("{}: Null reference provided", __FUNCTION__);
            return -1;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        Transition transition(a_time, a_interpolationMode, a_easeIn, a_easeOut);
        RE::BSTPoint2<float> offset({a_offsetPitch, a_offsetYaw});
        RotationPoint point(transition, PointType::kReference, RE::BSTPoint2<float>{}, offset, a_reference, a_isOffsetRelative);
        
        return static_cast<int>(state->m_timeline.AddRotationPoint(point));
    }

    bool TimelineManager::RemoveTranslationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        state->m_timeline.RemoveTranslationPoint(a_index);
        return true;
    }

    bool TimelineManager::RemoveRotationPoint(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, size_t a_index) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        state->m_timeline.RemoveRotationPoint(a_index);
        return true;
    }

    void TimelineManager::PlayTimeline(TimelineState* a_state) {
        if (!a_state || !a_state->m_isPlaybackRunning) {
            return;
        }
        
        if (a_state->m_timeline.GetTranslationPointCount() == 0 && a_state->m_timeline.GetRotationPointCount() == 0) {
            m_activeTimelineID = 0;
            a_state->m_isPlaybackRunning = false;
            return;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            log::error("{}: PlayerCamera not found during playback", __FUNCTION__);
            m_activeTimelineID = 0;
            a_state->m_isPlaybackRunning = false;
            return;
        }
        
        if (!playerCamera->IsInFreeCameraMode()) {
            m_activeTimelineID = 0;
            a_state->m_isPlaybackRunning = false;
            return;
        }
        
        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        
        if (!cameraState) {
            log::error("{}: FreeCameraState not found during playback", __FUNCTION__);
            m_activeTimelineID = 0;
            a_state->m_isPlaybackRunning = false;
            return;
        }
        
        float deltaTime = _ts_SKSEFunctions::GetRealTimeDeltaTime() * a_state->m_playbackSpeed;
        a_state->m_timeline.UpdatePlayback(deltaTime);
        
        // Apply global easing
        float sampleTime = a_state->m_timeline.GetPlaybackTime();
        if (a_state->m_globalEaseIn || a_state->m_globalEaseOut) {
            float timelineDuration = a_state->m_timeline.GetDuration();
            
            if (timelineDuration > 0.0f) {
                float linearProgress = std::clamp(sampleTime / timelineDuration, 0.0f, 1.0f);
                float easedProgress = _ts_SKSEFunctions::ApplyEasing(linearProgress, a_state->m_globalEaseIn, a_state->m_globalEaseOut);
                sampleTime = easedProgress * timelineDuration;
            }
        }
        
        // Get interpolated points
        cameraState->translation = a_state->m_timeline.GetTranslation(sampleTime);
        RE::BSTPoint2<float> rotation = a_state->m_timeline.GetRotation(sampleTime);
        
        // Handle user rotation (uses per-timeline m_allowUserRotation and global m_rotationOffset/m_userTurning)
        if (m_userTurning && a_state->m_allowUserRotation) {
            m_rotationOffset.x = _ts_SKSEFunctions::NormalRelativeAngle(cameraState->rotation.x - rotation.x);
            m_rotationOffset.y = _ts_SKSEFunctions::NormalRelativeAngle(cameraState->rotation.y - rotation.y);
            m_userTurning = false;
        } else {
            cameraState->rotation.x = _ts_SKSEFunctions::NormalRelativeAngle(rotation.x + m_rotationOffset.x);
            cameraState->rotation.y = _ts_SKSEFunctions::NormalRelativeAngle(rotation.y + m_rotationOffset.y);
        }
        
        // Check if timeline completed
        if (!a_state->m_timeline.IsPlaying()) {
            // Call StopPlayback to properly cleanup (exit free camera, restore UI, etc.)
            size_t timelineID = a_state->m_id;
            StopPlayback(timelineID);
        }
    }

    bool TimelineManager::ClearTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, bool a_notifyUser) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        if (state->m_isRecording) {
            return false;
        }
        
        if (a_notifyUser) {
            RE::DebugNotification("Clearing camera path...");
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
        }
        
        state->m_timeline.ClearPoints();
        
        return true;
    }

    bool TimelineManager::StartPlayback(size_t a_timelineID, float a_speed, bool a_globalEaseIn, bool a_globalEaseOut, bool a_useDuration, float a_duration) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        // Check if any timeline is already active
        if (m_activeTimelineID != 0) {
            log::error("{}: Timeline {} is already active", __FUNCTION__, m_activeTimelineID);
            return false;
        }
        
        TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        // Validation checks
        if (state->m_timeline.GetTranslationPointCount() == 0 && state->m_timeline.GetRotationPointCount() == 0) {
            log::error("{}: Timeline {} has no points", __FUNCTION__, a_timelineID);
            return false;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            log::error("{}: PlayerCamera not available", __FUNCTION__);
            return false;
        }
        
        if (playerCamera->IsInFreeCameraMode()) {
            log::error("{}: Already in free camera mode", __FUNCTION__);
            return false;
        }
        
        float timelineDuration = state->m_timeline.GetDuration();
        if (timelineDuration <= 0.0f && !a_useDuration) {
            log::error("{}: Timeline duration is zero", __FUNCTION__);
            return false;
        }
        
        // Calculate playback speed
        if (a_useDuration) {
            if (a_duration <= 0.0f) {
                log::warn("{}: Invalid duration {}, defaulting to timeline duration", __FUNCTION__, a_duration);
                state->m_playbackDuration = timelineDuration;
                state->m_playbackSpeed = 1.0f;
            } else {
                state->m_playbackDuration = a_duration;
                state->m_playbackSpeed = timelineDuration / state->m_playbackDuration;
            }
        } else {
            if (a_speed <= 0.0f) {
                log::warn("{}: Invalid speed {}, defaulting to 1.0", __FUNCTION__, a_speed);
                state->m_playbackDuration = timelineDuration;
                state->m_playbackSpeed = 1.0f;
            } else {
                state->m_playbackDuration = timelineDuration / a_speed;
                state->m_playbackSpeed = a_speed;
            }
        }
        
        if (state->m_playbackDuration <= 0.0f) {
            log::error("{}: Playback duration is zero", __FUNCTION__);
            return false;
        }
        
        // Set playback parameters
        state->m_globalEaseIn = a_globalEaseIn;
        state->m_globalEaseOut = a_globalEaseOut;
        
        // Set as active timeline
        m_activeTimelineID = a_timelineID;
        state->m_isPlaybackRunning = true;
        m_rotationOffset = { 0.0f, 0.0f };  // Global rotation offset
        
        // Save pre-playback state
        if (playerCamera->currentState && 
            (playerCamera->currentState->id == RE::CameraState::kThirdPerson ||
             playerCamera->currentState->id == RE::CameraState::kMount ||
             playerCamera->currentState->id == RE::CameraState::kDragon)) {
            
            RE::ThirdPersonState* cameraState = static_cast<RE::ThirdPersonState*>(playerCamera->currentState.get());
            if (cameraState) {
                m_lastFreeRotation = cameraState->freeRotation;
            }
        }
        
        // Initialize timeline playback
        state->m_timeline.ResetPlayback();
        state->m_timeline.StartPlayback();
        
        // Handle UI visibility
        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            m_isShowingMenus = ui->IsShowingMenus();
            ui->ShowMenus(state->m_showMenusDuringPlayback);
        }
        
        // Enter free camera mode
        playerCamera->ToggleFreeCameraMode(false);
        
        log::info("{}: Started playback on timeline {}", __FUNCTION__, a_timelineID);
        return true;
    }

    int TimelineManager::GetTranslationPointCount(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return -1;
        }
        
        return static_cast<int>(state->m_timeline.GetTranslationPointCount());
    }

    int TimelineManager::GetRotationPointCount(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return -1;
        }
        
        return static_cast<int>(state->m_timeline.GetRotationPointCount());
    }

    

    bool TimelineManager::StopPlayback(size_t a_timelineID) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        if (!state->m_isPlaybackRunning) {
            log::warn("{}: Timeline {} is not playing", __FUNCTION__, a_timelineID);
            return false;
        }
        
        if (m_activeTimelineID != a_timelineID) {
            log::error("{}: Timeline {} is not the active timeline", __FUNCTION__, a_timelineID);
            return false;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (playerCamera && playerCamera->IsInFreeCameraMode()) {
            playerCamera->ToggleFreeCameraMode(true);
            
            auto* ui = RE::UI::GetSingleton();
            if (ui) {
                ui->ShowMenus(m_isShowingMenus);
            }
            
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
        
        // Clear active state
        m_activeTimelineID = 0;
        state->m_isPlaybackRunning = false;
        m_rotationOffset = { 0.0f, 0.0f };
        
        log::info("{}: Stopped playback on timeline {}", __FUNCTION__, a_timelineID);
        return true;
    }

    void TimelineManager::SetUserTurning(bool a_turning) {
        m_userTurning = a_turning;
    }

    bool TimelineManager::PausePlayback(size_t a_timelineID) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        if (!state->m_isPlaybackRunning) {
            return false;
        }
        
        state->m_timeline.PausePlayback();
        return true;
    }

    bool TimelineManager::ResumePlayback(size_t a_timelineID) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        if (!state->m_isPlaybackRunning) {
            return false;
        }
        
        state->m_timeline.ResumePlayback();
        return true;
    }

    bool TimelineManager::IsPlaybackRunning(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        return state->m_isPlaybackRunning;
    }

    bool TimelineManager::IsRecording(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        return state->m_isRecording;
    }

    bool TimelineManager::IsPlaybackPaused(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        return state->m_timeline.IsPaused();
    }

    bool TimelineManager::AllowUserRotation(size_t a_timelineID, bool a_allow) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        state->m_allowUserRotation = a_allow;
        return true;
    }

    bool TimelineManager::IsUserRotationAllowed(size_t a_timelineID) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
        return state->m_allowUserRotation;
    }

    bool TimelineManager::AddTimelineFromFile(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle, const char* a_filePath, float a_timeOffset) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        if (state->m_isPlaybackRunning) {
            log::info("{}: Timeline modified during playback, stopping playback", __FUNCTION__);
            StopPlayback(a_timelineID);
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
        
        size_t translationPointCount = state->m_timeline.GetTranslationPointCount();
        size_t rotationPointCount = state->m_timeline.GetRotationPointCount();

        bool importTranslationSuccess = state->m_timeline.AddTranslationPathFromFile(file, a_timeOffset, 1.0f);
        
        // Rewind file to beginning for rotation import
        file.clear();  // Clear any error flags
        file.seekg(0, std::ios::beg);
        if (!file.good()) {
            log::error("{}: Failed to rewind file: {}", __FUNCTION__, fullPath.string());
            file.close();
            return false;
        }
        
        bool importRotationSuccess = state->m_timeline.AddRotationPathFromFile(file, a_timeOffset, degToRad);
        
        // Set playback mode and offset on timeline
        state->m_timeline.SetPlaybackMode(playbackMode);
        state->m_timeline.SetLoopTimeOffset(loopTimeOffset);
        
        file.close();
        
        if (!importTranslationSuccess) {
            log::error("{}: Failed to import translation points from {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        if (!importRotationSuccess) {
            log::error("{}: Failed to import rotation points from {}", __FUNCTION__, a_filePath);
            return false;
        }

        log::info("{}: Loaded {} translation and {} rotation points from {} to timeline {}", 
                  __FUNCTION__, state->m_timeline.GetTranslationPointCount() - translationPointCount, 
                  state->m_timeline.GetRotationPointCount() - rotationPointCount, a_filePath, a_timelineID);

        return true;
    }
    
    bool TimelineManager::ExportTimeline(size_t a_timelineID, const char* a_filePath) const {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        const TimelineState* state = GetTimeline(a_timelineID);
        if (!state) {
            return false;
        }
        
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
        file << "UseDegrees=1\n";
        
        // Get playback mode and offset from timeline
        int playbackModeInt = static_cast<int>(state->m_timeline.GetPlaybackMode());
        float loopTimeOffset = state->m_timeline.GetLoopTimeOffset();
        file << "PlaybackMode=" << playbackModeInt << "\n";
        file << "LoopTimeOffset=" << loopTimeOffset << "\n";
        file << "\n";
        
        float radToDeg = 180.0f / PI;

        bool exportTranslationSuccess = state->m_timeline.ExportTranslationPath(file, 1.0f);
        bool exportRotationSuccess = state->m_timeline.ExportRotationPath(file, radToDeg);
                
        file.close();

        if (!exportTranslationSuccess || !exportRotationSuccess) {
            log::error("{}: Failed to export points to {}", __FUNCTION__, a_filePath);
            return false;
        }
                
        if (!file.good()) {
            log::error("{}: Error occurred while writing file: {}", __FUNCTION__, a_filePath);
            return false;
        }
        
        log::info("{}: Exported {} translation and {} rotation points from timeline {} to {}", 
                  __FUNCTION__, state->m_timeline.GetTranslationPointCount(), 
                  state->m_timeline.GetRotationPointCount(),
                  a_timelineID,
                  a_filePath);
        return true;
    }

    size_t TimelineManager::RegisterTimeline(SKSE::PluginHandle a_pluginHandle) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        size_t newID = m_nextTimelineID.fetch_add(1);
        
        TimelineState state;
        state.m_id = newID;
        state.m_timeline = Timeline();
        state.m_ownerHandle = a_pluginHandle;
        
        state.m_ownerName = std::format("Plugin_{}", a_pluginHandle);
        
        m_timelines[newID] = std::move(state);
        
        log::info("{}: Timeline {} registered by plugin '{}' (handle {})", __FUNCTION__, newID, state.m_ownerName, a_pluginHandle);
        return newID;
    }

    bool TimelineManager::UnregisterTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) {
        std::lock_guard<std::recursive_mutex> lock(m_timelineMutex);
        
        TimelineState* state = GetTimeline(a_timelineID, a_pluginHandle);
        if (!state) {
            return false;
        }
        
        // Stop any active operations before unregistering
        if (m_activeTimelineID == a_timelineID) {
            if (state->m_isPlaybackRunning) {
                log::info("{}: Stopping playback before unregistering timeline {}", __FUNCTION__, a_timelineID);
                StopPlayback(a_timelineID);
            } else if (state->m_isRecording) {
                log::info("{}: Stopping recording before unregistering timeline {}", __FUNCTION__, a_timelineID);
                // With recursive_mutex, we can safely call StopRecording which will re-lock
                StopRecording(a_timelineID, a_pluginHandle);
            }
        }
        
        log::info("{}: Timeline {} unregistered (owner: {})", __FUNCTION__, a_timelineID, state->m_ownerName);
        m_timelines.erase(a_timelineID);
        return true;
    }

    TimelineState* TimelineManager::GetTimeline(size_t a_timelineID) {
        auto it = m_timelines.find(a_timelineID);
        if (it == m_timelines.end()) {
            log::error("{}: Timeline {} not found", __FUNCTION__, a_timelineID);
            return nullptr;
        }
        return &it->second;
    }

    const TimelineState* TimelineManager::GetTimeline(size_t a_timelineID) const {
        auto it = m_timelines.find(a_timelineID);
        if (it == m_timelines.end()) {
            log::error("{}: Timeline {} not found", __FUNCTION__, a_timelineID);
            return nullptr;
        }
        return &it->second;
    }

    TimelineState* TimelineManager::GetTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) {
        auto it = m_timelines.find(a_timelineID);
        if (it == m_timelines.end()) {
            log::error("{}: Timeline {} not found", __FUNCTION__, a_timelineID);
            return nullptr;
        }
        
        if (it->second.m_ownerHandle != a_pluginHandle) {
            log::error("{}: Plugin handle {} does not own timeline {} (owned by handle {})", 
                       __FUNCTION__, a_pluginHandle, a_timelineID, it->second.m_ownerHandle);
            return nullptr;
        }
        
        return &it->second;
    }

    const TimelineState* TimelineManager::GetTimeline(size_t a_timelineID, SKSE::PluginHandle a_pluginHandle) const {
        auto it = m_timelines.find(a_timelineID);
        if (it == m_timelines.end()) {
            log::error("{}: Timeline {} not found", __FUNCTION__, a_timelineID);
            return nullptr;
        }
        
        if (it->second.m_ownerHandle != a_pluginHandle) {
            log::error("{}: Plugin handle {} does not own timeline {} (owned by handle {})", 
                       __FUNCTION__, a_pluginHandle, a_timelineID, it->second.m_ownerHandle);
            return nullptr;
        }
        
        return &it->second;
    }

    void TimelineManager::RecordTimeline(TimelineState* a_state) {
        if (!a_state || !a_state->m_isRecording) {
            return;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        
        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            // Auto-stop if no longer in free camera
            m_activeTimelineID = 0;
            a_state->m_isRecording = false;
            return;
        }
        
        a_state->m_currentRecordingTime += _ts_SKSEFunctions::GetRealTimeDeltaTime();
        
        if (a_state->m_currentRecordingTime - a_state->m_lastRecordedPointTime >= m_recordingInterval) {
            // Capture camera position/rotation as kWorld points
            RE::NiPoint3 cameraPos = _ts_SKSEFunctions::GetCameraPos();
            RE::NiPoint3 cameraRot = _ts_SKSEFunctions::GetCameraRotation();
            
            Transition transTranslation(a_state->m_currentRecordingTime, InterpolationMode::kCubicHermite, false, false);
            TranslationPoint translationPoint(transTranslation, PointType::kWorld, cameraPos);
            a_state->m_timeline.AddTranslationPoint(translationPoint);
            
            Transition transRotation(a_state->m_currentRecordingTime, InterpolationMode::kCubicHermite, false, false);
            RotationPoint rotationPoint(transRotation, PointType::kWorld, RE::BSTPoint2<float>({cameraRot.x, cameraRot.z}));
            a_state->m_timeline.AddRotationPoint(rotationPoint);
            
            a_state->m_lastRecordedPointTime = a_state->m_currentRecordingTime;
        }
    }

    void TimelineManager::DrawTimeline(const TimelineState* a_state) {
        if (!a_state || !APIs::TrueHUD) {
            return;
        }
        
        if (a_state->m_timeline.GetTranslationPointCount() == 0 && a_state->m_timeline.GetRotationPointCount() == 0) {
            return;
        }
        
        if (a_state->m_isPlaybackRunning || a_state->m_isRecording) {
            return;
        }
        
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        
        if (!(playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree))) {
            return;
        }
        
        SetHUDMenuVisible(true);
        
        // Draw lines between translation points
        size_t pointCount = a_state->m_timeline.GetTranslationPointCount();
        for (size_t i = 0; i < pointCount - 1; ++i) {
            RE::NiPoint3 point1 = a_state->m_timeline.GetTranslationPointPosition(i);
            RE::NiPoint3 point2 = a_state->m_timeline.GetTranslationPointPosition(i + 1);
            APIs::TrueHUD->DrawLine(point1, point2);
        }
    }

} // namespace FCSE
