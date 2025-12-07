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

    void CameraPathManager::TraverseCamera() {
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            return;
        }

        if (!m_isTraversing || m_pathPoints.size() < 2) {
            return;
        }

        float deltaTime = _ts_SKSEFunctions::GetRealTimeDeltaTime();
        
        // Get current segment
        if (m_currentIndex >= m_pathPoints.size() - 1) {
            // Reached end
            StopTraversal();
            return;
        }

        const auto& nextPoint = m_pathPoints[m_currentIndex + 1];
        
        // Progress through segment based on time
        float segmentDuration = nextPoint.time;
        if (segmentDuration <= 0.0f) segmentDuration = 1.0f;
        
        m_currentSegmentProgress += deltaTime / segmentDuration;
        
        if (m_currentSegmentProgress >= 1.0f) {
            // Move to next segment
            m_currentIndex++;
            m_currentSegmentProgress = 0.0f;
        }

        auto currentPoint = GetCurrentPoint();

        RE::FreeCameraState* cameraState = nullptr;
        if (playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
        }
        if (!cameraState) {
            return;
        }

        cameraState->translation = currentPoint.position;
        cameraState->rotation = currentPoint.rotation;        
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
        // Draw lines between consecutive points
        for (size_t i = 0; i < m_pathPoints.size() - 1; ++i) {
            const auto& p1 = m_pathPoints[i];
            const auto& p2 = m_pathPoints[i + 1];
            
            // Draw line segment
            APIs::TrueHUD->DrawLine(p1.position, p2.position);
        }

        // Draw all points
        for (size_t i = 0; i < m_pathPoints.size(); ++i) {
            const auto& point = m_pathPoints[i];
            
            APIs::TrueHUD->DrawPoint(point.position, 10.f);
            RE::NiPoint3 direction = point.position;
            direction.x += 80.f * std::sin(point.rotation.y);
            direction.y += 80.f * std::cos(point.rotation.y);
            direction.z -= 80.f * std::sin(point.rotation.x);
            APIs::TrueHUD->DrawArrow(point.position, direction, 30.f);
        }
    }

    void CameraPathManager::AddPoint() {
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
            // FreeCamera activated via console command
            RE::NiPoint3 position = cameraState->translation;
            RE::BSTPoint2<float> rotation = cameraState->rotation;
log::info("FCSE - CameraPathManager: Adding point at current camera position: ({}, {}, {}), pitch: {}, yaw {}", position.x, position.y, position.z, 180.f/PI*rotation.x, 180.f/PI*rotation.y);

            float time = 1.0f;
        
            AddPoint(position, rotation, time);
        }
    }

    void CameraPathManager::AddPoint(const CameraPathPoint& point) {
        m_pathPoints.push_back(point);
    }

    void CameraPathManager::AddPoint(const RE::NiPoint3& pos, const RE::BSTPoint2<float>& rot, float time) {
            m_pathPoints.push_back({pos, rot, time});
    }

    void CameraPathManager::InsertPoint(size_t index, const CameraPathPoint& point) {
        if (index <= m_pathPoints.size()) {
            m_pathPoints.insert(m_pathPoints.begin() + index, point);
        }
    }

    void CameraPathManager::RemovePoint(size_t index) {
        if (index < m_pathPoints.size()) {
            m_pathPoints.erase(m_pathPoints.begin() + index);
        }
    }

    void CameraPathManager::ClearPath() {
        m_pathPoints.clear();
        StopTraversal();
    }

    const CameraPathPoint* CameraPathManager::GetPoint(size_t index) const {
        if (index < m_pathPoints.size()) {
            return &m_pathPoints[index];
        }
        return nullptr;
    }

    void CameraPathManager::StartTraversal() {
        if (m_pathPoints.size() < 2) {
            log::warn("FCSE - CameraPathManager: Need at least 2 points to traverse");
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
        m_currentIndex = 0;
        m_currentSegmentProgress = 0.0f;
        playerCamera->ToggleFreeCameraMode(false);
    }

    void CameraPathManager::StopTraversal() {
        m_isTraversing = false;
        m_currentIndex = 0;
        m_currentSegmentProgress = 0.0f;

        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        if (!playerCamera) {
            return;
        }
        if (!playerCamera->IsInFreeCameraMode()) {
            return;
        }
        
        playerCamera->ToggleFreeCameraMode(false);
    }
    CameraPathPoint CameraPathManager::GetCurrentPoint() const {
        switch (m_interpolationMode) {
            case InterpolationMode::Linear:
                return GetPointLinear();
            case InterpolationMode::CatmullRom:
                return GetPointCatmullRom();
            default:
                return GetPointLinear();
        }
    }

    CameraPathPoint CameraPathManager::GetPointLinear() const {
        if (m_pathPoints.empty()) {
            return {{0, 0, 0}, {0, 0}, 0.0f};
        }
        
        if (m_currentIndex >= m_pathPoints.size() - 1) {
            return m_pathPoints.back();
        }

        const auto& p1 = m_pathPoints[m_currentIndex];
        const auto& p2 = m_pathPoints[m_currentIndex + 1];
        
        // Linear interpolation
        float t = m_currentSegmentProgress;
        
        CameraPathPoint result;
        result.position = p1.position + (p2.position - p1.position) * t;
        
        // Interpolate rotation (shortest path for angles)
        float yawDelta = _ts_SKSEFunctions::NormalRelativeAngle(p2.rotation.y - p1.rotation.y);
        float pitchDelta = _ts_SKSEFunctions::NormalRelativeAngle(p2.rotation.x - p1.rotation.x);
        
        result.rotation.x = _ts_SKSEFunctions::NormalRelativeAngle(p1.rotation.x + pitchDelta * t);
        result.rotation.y = _ts_SKSEFunctions::NormalRelativeAngle(p1.rotation.y + yawDelta * t);
        result.time = p2.time;
        
        return result;
    }

    CameraPathPoint CameraPathManager::GetPointCatmullRom() const {
        if (m_pathPoints.empty()) {
            return {{0, 0, 0}, {0, 0}, 0.0f};
        }
        
        if (m_pathPoints.size() == 1) {
            return m_pathPoints[0];
        }
        
        if (m_currentIndex >= m_pathPoints.size() - 1) {
            return m_pathPoints.back();
        }

        // Get the four control points for Catmull-Rom spline
        // P0: previous point (or duplicate first point if at start)
        // P1: current point (start of segment)
        // P2: next point (end of segment)
        // P3: point after next (or duplicate last point if at end)
        
        size_t i0 = (m_currentIndex == 0) ? 0 : m_currentIndex - 1;
        size_t i1 = m_currentIndex;
        size_t i2 = m_currentIndex + 1;
        size_t i3 = (i2 >= m_pathPoints.size() - 1) ? i2 : i2 + 1;
        
        const auto& p0 = m_pathPoints[i0];
        const auto& p1 = m_pathPoints[i1];
        const auto& p2 = m_pathPoints[i2];
        const auto& p3 = m_pathPoints[i3];
        
        float t = m_currentSegmentProgress;
        float t2 = t * t;
        float t3 = t2 * t;
        
        CameraPathPoint result;
        
        // Catmull-Rom spline interpolation for position
        // Formula: 0.5 * ((2*P1) + (-P0 + P2)*t + (2*P0 - 5*P1 + 4*P2 - P3)*t^2 + (-P0 + 3*P1 - 3*P2 + P3)*t^3)
        result.position.x = 0.5f * (
            (2.0f * p1.position.x) +
            (-p0.position.x + p2.position.x) * t +
            (2.0f * p0.position.x - 5.0f * p1.position.x + 4.0f * p2.position.x - p3.position.x) * t2 +
            (-p0.position.x + 3.0f * p1.position.x - 3.0f * p2.position.x + p3.position.x) * t3
        );
        
        result.position.y = 0.5f * (
            (2.0f * p1.position.y) +
            (-p0.position.y + p2.position.y) * t +
            (2.0f * p0.position.y - 5.0f * p1.position.y + 4.0f * p2.position.y - p3.position.y) * t2 +
            (-p0.position.y + 3.0f * p1.position.y - 3.0f * p2.position.y + p3.position.y) * t3
        );
        
        result.position.z = 0.5f * (
            (2.0f * p1.position.z) +
            (-p0.position.z + p2.position.z) * t +
            (2.0f * p0.position.z - 5.0f * p1.position.z + 4.0f * p2.position.z - p3.position.z) * t2 +
            (-p0.position.z + 3.0f * p1.position.z - 3.0f * p2.position.z + p3.position.z) * t3
        );
        
        // Smooth rotation interpolation using Catmull-Rom on angles
        // Need to handle angle wrapping carefully
        
        // Pitch interpolation
        float pitch0 = p0.rotation.x;
        float pitch1 = p1.rotation.x;
        float pitch2 = p2.rotation.x;
        float pitch3 = p3.rotation.x;
        
        // Normalize deltas to shortest path
        pitch2 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(pitch2 - pitch1);
        pitch0 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(pitch0 - pitch1);
        pitch3 = pitch2 + _ts_SKSEFunctions::NormalRelativeAngle(pitch3 - pitch2);
        
        result.rotation.x = 0.5f * (
            (2.0f * pitch1) +
            (-pitch0 + pitch2) * t +
            (2.0f * pitch0 - 5.0f * pitch1 + 4.0f * pitch2 - pitch3) * t2 +
            (-pitch0 + 3.0f * pitch1 - 3.0f * pitch2 + pitch3) * t3
        );
        result.rotation.x = _ts_SKSEFunctions::NormalRelativeAngle(result.rotation.x);
        
        // Yaw interpolation
        float yaw0 = p0.rotation.y;
        float yaw1 = p1.rotation.y;
        float yaw2 = p2.rotation.y;
        float yaw3 = p3.rotation.y;
        
        // Normalize deltas to shortest path
        yaw2 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(yaw2 - yaw1);
        yaw0 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(yaw0 - yaw1);
        yaw3 = yaw2 + _ts_SKSEFunctions::NormalRelativeAngle(yaw3 - yaw2);
        
        result.rotation.y = 0.5f * (
            (2.0f * yaw1) +
            (-yaw0 + yaw2) * t +
            (2.0f * yaw0 - 5.0f * yaw1 + 4.0f * yaw2 - yaw3) * t2 +
            (-yaw0 + 3.0f * yaw1 - 3.0f * yaw2 + yaw3) * t3
        );
        result.rotation.y = _ts_SKSEFunctions::NormalRelativeAngle(result.rotation.y);
        
        result.time = p2.time;
        
        return result;
    }

} // namespace FCSE