#include "CameraPathManager.h"
#include "_ts_SKSEFunctions.h"
#include "APIManager.h"

namespace FCSE {

    void CameraPathManager::Update() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        if (playerCamera->IsInFreeCameraMode()) {
            DrawPath();
        }

        TraverseCamera();
    }

    void CameraPathManager::ProcessTimeline(TimelineType a_timelineType, float a_deltaTime) {
        size_t& currentIndex = (a_timelineType == TimelineType::Translation) 
            ? m_currentTranslationIndex 
            : m_currentRotationIndex;
        
        float& progress = (a_timelineType == TimelineType::Translation) 
            ? m_transitionTranslateProgress 
            : m_transitionRotateProgress;
        
        if (currentIndex >= m_pathPoints.size()) {
            return;
        }
        
        const auto& currentPoint = m_pathPoints[currentIndex];
        const Transition& transition = (a_timelineType == TimelineType::Translation) 
            ? currentPoint.m_transitionTranslate 
            : currentPoint.m_transitionRotate;
        
        if (transition.m_interpolationType == InterpolationType::kOn) {
            // Interpolating - progress through segment
            float segmentDuration = transition.m_time;
            if (segmentDuration > 0.0f) {
                progress += a_deltaTime / segmentDuration;
            } else {
                progress = 1.0f;
            }
            
            if (progress >= 1.0f) {
                // Completed this segment
                currentIndex++;
                progress = 0.0f;
            }
        } else if (transition.m_interpolationType == InterpolationType::kOff) {
            // Instant set - move to next immediately
            currentIndex++;
            progress = 0.0f;
        } else {
            // kInvalid - skip this point
            currentIndex++;
            progress = 0.0f;
        }
    }

    void CameraPathManager::TraverseCamera() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            return;
        }

        if (!m_isTraversing || m_pathPoints.empty()) {
            return;
        }

        float deltaTime = _ts_SKSEFunctions::GetRealTimeDeltaTime();
        
        ProcessTimeline(TimelineType::Translation, deltaTime);

        ProcessTimeline(TimelineType::Rotation, deltaTime);
        
        // Check if both timelines are complete
        if (m_currentTranslationIndex >= m_pathPoints.size() && 
            m_currentRotationIndex >= m_pathPoints.size()) {
            StopTraversal();
            return;
        }

        // Get interpolated values
        RE::NiPoint3 currentPosition = GetCurrentPosition();
        RE::BSTPoint2<float> currentRotation = GetCurrentRotation();

        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        if (!cameraState) {
            return;
        }

        cameraState->translation = currentPosition;
        cameraState->rotation = currentRotation;        
    }

    void CameraPathManager::SetHUDMenuVisible(bool a_visible) {
        if (!APIs::TrueHUD || m_pathPoints.empty()) {
            return;
        }

        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto trueHUDMenu = ui->GetMenu<RE::IMenu>("TrueHUD");
            if (trueHUDMenu && trueHUDMenu->uiMovie) {
                trueHUDMenu->uiMovie->SetVisible(a_visible);
            }
        }        
    }

    void CameraPathManager::DrawPath() {
        if (!APIs::TrueHUD || m_pathPoints.empty() || m_isTraversing) {
            return;
        }

        SetHUDMenuVisible(true);
        
        // Draw translation path (lines between translation points)
        for (size_t i = 0; i < m_pathPoints.size() - 1; ++i) {
            if (m_pathPoints[i].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                // Find next translation point
                for (size_t j = i + 1; j < m_pathPoints.size(); ++j) {
                    if (m_pathPoints[j].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                        APIs::TrueHUD->DrawLine(m_pathPoints[i].m_position, m_pathPoints[j].m_position);
                        break;
                    }
                }
            }
        }

        // Draw all points with arrows showing rotation
        for (size_t i = 0; i < m_pathPoints.size(); ++i) {
            const auto& point = m_pathPoints[i];
            
            if (point.m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                APIs::TrueHUD->DrawPoint(point.m_position, 10.f);
            }

            if (point.m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                APIs::TrueHUD->DrawPoint(point.m_position, 5.f, 0.0f, 0xFFFFFF00); // Yellow point
            }
            
            if (point.m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                RE::NiPoint3 direction = point.m_position;
                direction.x += 80.f * std::sin(point.m_rotation.y);
                direction.y += 80.f * std::cos(point.m_rotation.y);
                direction.z -= 80.f * std::sin(point.m_rotation.x);
                APIs::TrueHUD->DrawArrow(point.m_position, direction, 30.f,0.0f, 0xFFFFFF00, 5.0f); // Yellow arrow
            }
        }
    }

    void CameraPathManager::AddPathPoint(TimelineType a_type) {
        AddPathPoint(a_type, 1.0f, true, true);
    }

    void CameraPathManager::AddPathPoint(TimelineType a_type, float a_time, bool a_easeIn, bool a_easeOut) {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }

        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        if (!cameraState) {
            return;
        }  

        if (playerCamera->IsInFreeCameraMode()) {
            RE::NiPoint3 currentPos = cameraState->translation;
            RE::BSTPoint2<float> currentRot = cameraState->rotation;
            
            Transition translationTrans;
            Transition rotationTrans;
            
            if (a_type == TimelineType::Translation) {
                translationTrans = Transition(InterpolationType::kOn, a_time, a_easeIn, a_easeOut);
                rotationTrans = Transition(InterpolationType::kInvalid, 0.0f, false, false);
log::info("FCSE - CameraPathManager: Adding translation point at: ({}, {}, {}), time={}, easeIn={}, easeOut={}", 
currentPos.x, currentPos.y, currentPos.z, a_time, a_easeIn, a_easeOut);
            } else {
                translationTrans = Transition(InterpolationType::kInvalid, 0.0f, false, false);
                rotationTrans = Transition(InterpolationType::kOn, a_time, a_easeIn, a_easeOut);
log::info("FCSE - CameraPathManager: Adding rotation point, pitch: {}, yaw: {}, time={}, easeIn={}, easeOut={}", 
180.f/PI*currentRot.x, 180.f/PI*currentRot.y, a_time, a_easeIn, a_easeOut);
            }
            
            CameraPathPoint point(translationTrans, rotationTrans, currentPos, currentRot);
            AddPoint(point);
        }
    }

    void CameraPathManager::AddPoint(const CameraPathPoint& a_point) {
        m_pathPoints.push_back(a_point);
    }

    void CameraPathManager::InsertPoint(size_t a_index, const CameraPathPoint& a_point) {
        if (a_index <= m_pathPoints.size()) {
            m_pathPoints.insert(m_pathPoints.begin() + a_index, a_point);
        }
    }

    void CameraPathManager::RemovePoint(size_t a_index) {
        if (a_index < m_pathPoints.size()) {
            m_pathPoints.erase(m_pathPoints.begin() + a_index);
        }
    }

    void CameraPathManager::ClearPath() {
        m_pathPoints.clear();
        StopTraversal();
    }

    const CameraPathPoint* CameraPathManager::GetPoint(size_t a_index) const {
        if (a_index < m_pathPoints.size()) {
            return &m_pathPoints[a_index];
        }
        return nullptr;
    }

    void CameraPathManager::StartTraversal() {
        if (m_pathPoints.empty()) {
            log::warn("FCSE - CameraPathManager: Need at least 1 point to traverse");
            return;
        }

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (playerCamera->IsInFreeCameraMode()) {
            return;
        }
        
        m_isTraversing = true;
        m_currentTranslationIndex = 0;
        m_transitionTranslateProgress = 0.0f;
        m_currentRotationIndex = 0;
        m_transitionRotateProgress = 0.0f;
        playerCamera->ToggleFreeCameraMode(false);
    }

    void CameraPathManager::StopTraversal() {
        m_isTraversing = false;
        m_currentTranslationIndex = 0;
        m_transitionTranslateProgress = 0.0f;
        m_currentRotationIndex = 0;
        m_transitionRotateProgress = 0.0f;

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            return;
        }
        
        playerCamera->ToggleFreeCameraMode(false);
    }
    RE::NiPoint3 CameraPathManager::GetCurrentPosition() const {
        switch (m_interpolationMode) {
            case InterpolationMode::kNone:
                if (m_currentTranslationIndex < m_pathPoints.size()) {
                    return m_pathPoints[m_currentTranslationIndex].m_position;
                }
                return {0.f, 0.f, 0.f};
            case InterpolationMode::kLinear:
                return GetTranslationLinear();
            case InterpolationMode::kCatmullRom:
                return GetTranslationCubicHermite();
            default:
                return GetTranslationLinear();
        }
    }
    
    RE::BSTPoint2<float> CameraPathManager::GetCurrentRotation() const {
        switch (m_interpolationMode) {
            case InterpolationMode::kNone:
                if (m_currentRotationIndex < m_pathPoints.size()) {
                    return m_pathPoints[m_currentRotationIndex].m_rotation;
                }
                return {0.f, 0.f};
            case InterpolationMode::kLinear:
                return GetRotationLinear();
            case InterpolationMode::kCatmullRom:
                return GetRotationCubicHermite();
            default:
                return GetRotationLinear();
        }
    }

    RE::NiPoint3 CameraPathManager::GetTranslationLinear() const {
        if (m_pathPoints.empty()) {
            return {0.f, 0.f, 0.f};
        }
        
        size_t currentIdx = m_currentTranslationIndex;
        if (currentIdx >= m_pathPoints.size()) {
            currentIdx = m_pathPoints.size() - 1;
        }
        
        const auto& currentPoint = m_pathPoints[currentIdx];
        const auto& currentPos = currentPoint.m_position;
        
        if (currentPoint.m_transitionTranslate.m_interpolationType == InterpolationType::kOff || currentIdx == 0) {
            return currentPos;
        }
        
        // Find previous translation point
        size_t prevIdx = currentIdx;
        for (size_t i = currentIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                prevIdx = i - 1;
                break;
            }
        }
        
        if (prevIdx == currentIdx) {
            return currentPos;
        }
        
        const auto& prevPos = m_pathPoints[prevIdx].m_position;
        
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionTranslateProgress,
                             currentPoint.m_transitionTranslate.m_easeIn,
                             currentPoint.m_transitionTranslate.m_easeOut);
        
        return prevPos + (currentPos - prevPos) * t;
    }
    
    RE::BSTPoint2<float> CameraPathManager::GetRotationLinear() const {
        if (m_pathPoints.empty()) {
            return {0.f, 0.f};
        }
        
        size_t currentIdx = m_currentRotationIndex;
        if (currentIdx >= m_pathPoints.size()) {
            currentIdx = m_pathPoints.size() - 1;
        }
        
        const auto& currentPoint = m_pathPoints[currentIdx];
        const auto& currentRot = currentPoint.m_rotation;
        
        if (currentPoint.m_transitionRotate.m_interpolationType == InterpolationType::kOff || currentIdx == 0) {
            return currentRot;
        }
        
        // Find previous rotation point
        size_t prevIdx = currentIdx;
        for (size_t i = currentIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                prevIdx = i - 1;
                break;
            }
        }
        
        if (prevIdx == currentIdx) {
            return currentRot;
        }
        
        const auto& prevRot = m_pathPoints[prevIdx].m_rotation;
        
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionRotateProgress,
                             currentPoint.m_transitionRotate.m_easeIn,
                             currentPoint.m_transitionRotate.m_easeOut);
        
        // Interpolate rotation with angle wrapping
        float yawDelta = _ts_SKSEFunctions::NormalRelativeAngle(currentRot.y - prevRot.y);
        float pitchDelta = _ts_SKSEFunctions::NormalRelativeAngle(currentRot.x - prevRot.x);
        
        RE::BSTPoint2<float> result;
        result.x = _ts_SKSEFunctions::NormalRelativeAngle(prevRot.x + pitchDelta * t);
        result.y = _ts_SKSEFunctions::NormalRelativeAngle(prevRot.y + yawDelta * t);
        return result;
    }

    RE::NiPoint3 CameraPathManager::GetTranslationCubicHermite() const {
        if (m_pathPoints.empty()) {
            return {0.f, 0.f, 0.f};
        }
        
        if (m_pathPoints.size() == 1) {
            return m_pathPoints[0].m_position;
        }
        
        size_t currentIdx = m_currentTranslationIndex;
        if (currentIdx >= m_pathPoints.size()) {
            return m_pathPoints.back().m_position;
        }
        
        const auto& currentPoint = m_pathPoints[currentIdx];
        const auto& currentPos = currentPoint.m_position;
        
        if (currentPoint.m_transitionTranslate.m_interpolationType == InterpolationType::kOff || currentIdx == 0) {
            return currentPos;
        }
        
        // Find previous translation point
        size_t prevIdx = currentIdx;
        for (size_t i = currentIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                prevIdx = i - 1;
                break;
            }
        }
        
        if (prevIdx == currentIdx) {
            return currentPos;
        }
        
        const auto& p1 = m_pathPoints[prevIdx].m_position;
        const auto& p2 = currentPoint.m_position;
        
        // Compute tangents using Cubic Hermite spline approach
        // Tangent at p1: direction from point before to point after
        RE::NiPoint3 m1;
        size_t beforePrev = prevIdx;
        for (size_t i = prevIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                beforePrev = i - 1;
                break;
            }
        }
        
        if (beforePrev < prevIdx) {
            // Have point before: tangent = (p2 - p0) / 2
            const auto& p0 = m_pathPoints[beforePrev].m_position;
            m1 = (p2 - p0) * 0.5f;
        } else {
            // First point: tangent = p2 - p1
            m1 = p2 - p1;
        }
        
        // Tangent at p2: direction from point before to point after
        RE::NiPoint3 m2;
        size_t afterCurrent = currentIdx;
        for (size_t i = currentIdx + 1; i < m_pathPoints.size(); ++i) {
            if (m_pathPoints[i].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                afterCurrent = i;
                break;
            }
        }
        
        if (afterCurrent > currentIdx) {
            // Have point after: tangent = (p3 - p1) / 2
            const auto& p3 = m_pathPoints[afterCurrent].m_position;
            m2 = (p3 - p1) * 0.5f;
        } else {
            // Last point: tangent = p2 - p1
            m2 = p2 - p1;
        }
        
        // If points are identical, tangent should be zero to prevent overshoot
        float distSq = (p2.x - p1.x) * (p2.x - p1.x) + 
                       (p2.y - p1.y) * (p2.y - p1.y) + 
                       (p2.z - p1.z) * (p2.z - p1.z);
        if (distSq < 0.001f) {
            // Points are essentially the same, use zero tangents
            m1 = {0.f, 0.f, 0.f};
            m2 = {0.f, 0.f, 0.f};
        }
        
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionTranslateProgress,
                             currentPoint.m_transitionTranslate.m_easeIn,
                             currentPoint.m_transitionTranslate.m_easeOut);
        
        // Cubic Hermite interpolation
        float t2 = t * t;
        float t3 = t2 * t;
        
        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;   // basis function for p1
        float h10 = t3 - 2.0f * t2 + t;              // basis function for m1
        float h01 = -2.0f * t3 + 3.0f * t2;          // basis function for p2
        float h11 = t3 - t2;                          // basis function for m2
        
        RE::NiPoint3 result;
        result.x = h00 * p1.x + h10 * m1.x + h01 * p2.x + h11 * m2.x;
        result.y = h00 * p1.y + h10 * m1.y + h01 * p2.y + h11 * m2.y;
        result.z = h00 * p1.z + h10 * m1.z + h01 * p2.z + h11 * m2.z;
        
        return result;
    }
    
    RE::BSTPoint2<float> CameraPathManager::GetRotationCubicHermite() const {
        if (m_pathPoints.empty()) {
            return {0.f, 0.f};
        }
        
        if (m_pathPoints.size() == 1) {
            return m_pathPoints[0].m_rotation;
        }
        
        size_t currentIdx = m_currentRotationIndex;
        if (currentIdx >= m_pathPoints.size()) {
            return m_pathPoints.back().m_rotation;
        }
        
        const auto& currentPoint = m_pathPoints[currentIdx];
        const auto& currentRot = currentPoint.m_rotation;
        
        if (currentPoint.m_transitionRotate.m_interpolationType == InterpolationType::kOff || currentIdx == 0) {
            return currentRot;
        }
        
        // Find previous rotation point
        size_t prevIdx = currentIdx;
        for (size_t i = currentIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                prevIdx = i - 1;
                break;
            }
        }
        
        if (prevIdx == currentIdx) {
            return currentRot;
        }
        
        const auto& r1 = m_pathPoints[prevIdx].m_rotation;
        const auto& r2 = currentPoint.m_rotation;
        
        // Normalize angles for interpolation
        float pitch1 = r1.x;
        float pitch2 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(r2.x - pitch1);
        float yaw1 = r1.y;
        float yaw2 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(r2.y - yaw1);
        
        // Compute tangents for Cubic Hermite spline
        // Tangent at r1
        float m1_pitch, m1_yaw;
        size_t beforePrev = prevIdx;
        for (size_t i = prevIdx; i > 0; --i) {
            if (m_pathPoints[i - 1].m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                beforePrev = i - 1;
                break;
            }
        }
        
        if (beforePrev < prevIdx) {
            const auto& r0 = m_pathPoints[beforePrev].m_rotation;
            float pitch0 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(r0.x - pitch1);
            float yaw0 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(r0.y - yaw1);
            m1_pitch = (pitch2 - pitch0) * 0.5f;
            m1_yaw = (yaw2 - yaw0) * 0.5f;
        } else {
            m1_pitch = pitch2 - pitch1;
            m1_yaw = yaw2 - yaw1;
        }
        
        // Tangent at r2
        float m2_pitch, m2_yaw;
        size_t afterCurrent = currentIdx;
        for (size_t i = currentIdx + 1; i < m_pathPoints.size(); ++i) {
            if (m_pathPoints[i].m_transitionRotate.m_interpolationType != InterpolationType::kInvalid) {
                afterCurrent = i;
                break;
            }
        }
        
        if (afterCurrent > currentIdx) {
            const auto& r3 = m_pathPoints[afterCurrent].m_rotation;
            float pitch3 = pitch2 + _ts_SKSEFunctions::NormalRelativeAngle(r3.x - r2.x);
            float yaw3 = yaw2 + _ts_SKSEFunctions::NormalRelativeAngle(r3.y - r2.y);
            m2_pitch = (pitch3 - pitch1) * 0.5f;
            m2_yaw = (yaw3 - yaw1) * 0.5f;
        } else {
            m2_pitch = pitch2 - pitch1;
            m2_yaw = yaw2 - yaw1;
        }
        
        // If rotations are identical, use zero tangents
        float pitchDiff = std::abs(_ts_SKSEFunctions::NormalRelativeAngle(r2.x - r1.x));
        float yawDiff = std::abs(_ts_SKSEFunctions::NormalRelativeAngle(r2.y - r1.y));
        if (pitchDiff < 0.001f && yawDiff < 0.001f) {
            m1_pitch = 0.f;
            m1_yaw = 0.f;
            m2_pitch = 0.f;
            m2_yaw = 0.f;
        }
        
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionRotateProgress,
                             currentPoint.m_transitionRotate.m_easeIn,
                             currentPoint.m_transitionRotate.m_easeOut);
        
        // Cubic Hermite interpolation
        float t2 = t * t;
        float t3 = t2 * t;
        
        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        float h10 = t3 - 2.0f * t2 + t;
        float h01 = -2.0f * t3 + 3.0f * t2;
        float h11 = t3 - t2;
        
        float resultPitch = h00 * pitch1 + h10 * m1_pitch + h01 * pitch2 + h11 * m2_pitch;
        float resultYaw = h00 * yaw1 + h10 * m1_yaw + h01 * yaw2 + h11 * m2_yaw;
        
        RE::BSTPoint2<float> result;
        result.x = _ts_SKSEFunctions::NormalRelativeAngle(resultPitch);
        result.y = _ts_SKSEFunctions::NormalRelativeAngle(resultYaw);
        return result;
    }

} // namespace FCSE