#include "CameraPathManager.h"
#include "_ts_SKSEFunctions.h"
#include "APIManager.h"

namespace FCSE {

    void CameraPathManager::Update() {
        DrawPath();

        TraverseCamera();

        RecordPath();
    }

    void CameraPathManager::StartRecording() {
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

        ClearPath(false);

        AddTranslationPoint(m_currentRecordingTime, true, false);
        AddRotationPoint(m_currentRecordingTime, true, false);
    }

    void CameraPathManager::StopRecording() {
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

    void CameraPathManager::RecordPath() {
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

    template<typename PointType>
    void CameraPathManager::ProcessTimeline(const std::vector<PointType>& a_points, size_t& a_currentIndex, float& a_progress, Transition PointType::* a_transitionMember) {
        
        if (a_points.empty()) {
            return;
        }
        
        // Find the appropriate point index for current time
        size_t targetIndex = a_currentIndex;
        for (size_t i = a_currentIndex; i < a_points.size(); ++i) {
            const Transition& transition = a_points[i].*a_transitionMember;
            if (m_currentTraversalTime <= transition.m_time) {
                targetIndex = i;
                break;
            }
            targetIndex = i + 1;
        }
        
        if (targetIndex >= a_points.size()) {
            a_currentIndex = a_points.size();
            a_progress = 1.0f;
            return;
        }
        
        a_currentIndex = targetIndex;
        const auto& currentPoint = a_points[targetIndex];
        const Transition& currentTransition = currentPoint.*a_transitionMember;
        
        // Calculate progress within current segment
        float prevTime = 0.0f;
        if (targetIndex > 0) {
            const Transition& prevTransition = a_points[targetIndex - 1].*a_transitionMember;
            prevTime = prevTransition.m_time;
        }
        
        float segmentDuration = currentTransition.m_time - prevTime;
        if (segmentDuration > 0.0f) {
            a_progress = (m_currentTraversalTime - prevTime) / segmentDuration;
            a_progress = std::clamp(a_progress, 0.0f, 1.0f);
        } else {
            a_progress = 1.0f;
        }
    }

    void CameraPathManager::ProcessTimeline(TimelineType a_timelineType) {
        if (a_timelineType == TimelineType::kTranslation) {
            ProcessTimeline(m_translationPoints, m_currentTranslationIndex, m_transitionTranslateProgress, &CameraTranslationPoint::m_transitionTranslate);
        } else {
            ProcessTimeline(m_rotationPoints, m_currentRotationIndex, m_transitionRotateProgress, &CameraRotationPoint::m_transitionRotate);
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

        if (m_isRecording) {
            return;
        }

        if (!m_isTraversing || (m_translationPoints.empty() && m_rotationPoints.empty())) {
            return;
        }

        m_currentTraversalTime += _ts_SKSEFunctions::GetRealTimeDeltaTime();

        ProcessTimeline(TimelineType::kTranslation);

        ProcessTimeline(TimelineType::kRotation);
        
        // Check if both timelines are complete
        if (m_currentTranslationIndex >= m_translationPoints.size() && 
            m_currentRotationIndex >= m_rotationPoints.size()) {
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
        if (!APIs::TrueHUD || (m_translationPoints.empty() && m_rotationPoints.empty())) {
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
        if (!APIs::TrueHUD || (m_translationPoints.empty() && m_rotationPoints.empty()) || m_isTraversing) {
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
        for (size_t i = 0; i < m_translationPoints.size() - 1; ++i) {
            if (m_translationPoints[i].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                // Find next translation point
                for (size_t j = i + 1; j < m_translationPoints.size(); ++j) {
                    if (m_translationPoints[j].m_transitionTranslate.m_interpolationType != InterpolationType::kInvalid) {
                        APIs::TrueHUD->DrawLine(m_translationPoints[i].m_position, m_translationPoints[j].m_position);
                        break;
                    }
                }
            }
        }
/* TODO: Rotation points - where to locate them?
        // Draw all points with arrows showing rotation
        for (size_t i = 0; i < m_rotationPoints.size(); ++i) {
            const auto& point = m_rotationPoints[i];
            
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
        } */
    }

    void CameraPathManager::AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut) {
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
            AddPoint(&CameraPathManager::AddTranslationPoint, cameraState->translation, a_time, a_easeIn, a_easeOut);

            if (!m_isRecording) {
                RE::DebugNotification("Added camera translation point...");
            }

log::info("FCSE - Added TranslationPoint point at absolute time {}, pos=({},{},{})",
a_time, cameraState->translation.x, cameraState->translation.y, cameraState->translation.z);
        }
    }    

    void CameraPathManager::AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut) {
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
            AddPoint(&CameraPathManager::AddRotationPoint, cameraState->rotation, a_time, a_easeIn, a_easeOut);
            
            if (!m_isRecording) {
                RE::DebugNotification("Added camera rotation point...");
            }

log::info("FCSE - Added RotationPoint point at absolute time {}, rot=({},{})",
a_time, 180.f/PI*cameraState->rotation.x, 180.f/PI*cameraState->rotation.y);
         }
    }


    template<typename PointType, typename ValueType>
    void CameraPathManager::AddPoint(
        void (CameraPathManager::*a_addFunction)(const PointType&),
        const ValueType& a_value,
        float a_time,
        bool a_easeIn,
        bool a_easeOut)
    {
        Transition transition(InterpolationType::kOn, a_time, a_easeIn, a_easeOut);
        PointType point(transition, a_value);
        (this->*a_addFunction)(point);
    }

    template<typename PointType>
    void CameraPathManager::InsertPointSorted(
        std::vector<PointType>& a_points,
        Transition PointType::* a_transitionMember,
        const PointType& a_point)
    {
        // Validate and clamp time value to non-negative
        PointType validatedPoint = a_point;
        Transition& transition = validatedPoint.*a_transitionMember;
        
        if (transition.m_time < 0.0f) {
            transition.m_time = 0.0f;
        }
        
        // Insert point in sorted order by time
        float pointTime = transition.m_time;
        auto insertPos = std::lower_bound(a_points.begin(), a_points.end(), pointTime,
            [a_transitionMember](const PointType& point, float time) {
                const Transition& trans = point.*a_transitionMember;
                return trans.m_time < time;
            });
        
        a_points.insert(insertPos, validatedPoint);
    }

    void CameraPathManager::AddTranslationPoint(const CameraTranslationPoint& a_point) {
        InsertPointSorted(m_translationPoints, &CameraTranslationPoint::m_transitionTranslate, a_point);
    }

    void CameraPathManager::AddRotationPoint(const CameraRotationPoint& a_point) {
        InsertPointSorted(m_rotationPoints, &CameraRotationPoint::m_transitionRotate, a_point);
    }

    void CameraPathManager::InsertTranslationPoint(size_t a_index, const CameraTranslationPoint& a_point) {
        if (a_index <= m_translationPoints.size()) {
            m_translationPoints.insert(m_translationPoints.begin() + a_index, a_point);
        }
    }

    void CameraPathManager::InsertRotationPoint(size_t a_index, const CameraRotationPoint& a_point) {
        if (a_index <= m_rotationPoints.size()) {
            m_rotationPoints.insert(m_rotationPoints.begin() + a_index, a_point);
        }
    }

    void CameraPathManager::RemoveTranslationPoint(size_t a_index) {
        if (a_index < m_translationPoints.size()) {
            m_translationPoints.erase(m_translationPoints.begin() + a_index);
        }
    }

    void CameraPathManager::RemoveRotationPoint(size_t a_index) {
        if (a_index < m_rotationPoints.size()) {
            m_rotationPoints.erase(m_rotationPoints.begin() + a_index);
        }
    }

    void CameraPathManager::ClearPath(bool a_notify) {
        if (m_isRecording) {
            return;
        }

        if (a_notify) {
            RE::DebugNotification("Clearing camera path...");
        }
        
        m_translationPoints.clear();
        m_rotationPoints.clear();
        StopTraversal();
    }

    const CameraTranslationPoint* CameraPathManager::GetTranslationPoint(size_t a_index) const {
        if (a_index < m_translationPoints.size()) {
            return &m_translationPoints[a_index];
        }
        return nullptr;
    }

    const CameraRotationPoint* CameraPathManager::GetRotationPoint(size_t a_index) const {
        if (a_index < m_rotationPoints.size()) {
            return &m_rotationPoints[a_index];
        }
        return nullptr;
    }

    void CameraPathManager::StartTraversal() {
        if (m_translationPoints.empty() && m_rotationPoints.empty()) {
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
        m_currentTraversalTime = 0.0f;
        m_currentTranslationIndex = 0;
        m_transitionTranslateProgress = 0.0f;
        m_currentRotationIndex = 0;
        m_transitionRotateProgress = 0.0f;
        playerCamera->ToggleFreeCameraMode(false);
    }

    void CameraPathManager::StopTraversal() {

        if (m_isTraversing) {
            auto* playerCamera = RE::PlayerCamera::GetSingleton();
            if (!playerCamera) {
                return;
            }
            if (!playerCamera->IsInFreeCameraMode()) {
                return;
            }
            
            playerCamera->ToggleFreeCameraMode(false);
        }

        m_isTraversing = false;
        m_currentTraversalTime = 0.0f;
        m_currentTranslationIndex = 0;
        m_transitionTranslateProgress = 0.0f;
        m_currentRotationIndex = 0;
        m_transitionRotateProgress = 0.0f;

    }
    
    // Helper template function to set values in INI file
    template <typename T>
    static bool SetValueInINI(CSimpleIniA& a_ini, const std::string& a_section, const std::string& a_key, T a_value) {
        if constexpr (std::is_same_v<T, bool>) {
            return a_ini.SetBoolValue(a_section.c_str(), a_key.c_str(), a_value) == SI_OK;
        } else if constexpr (std::is_same_v<T, double> || std::is_same_v<T, float>) {
            return a_ini.SetDoubleValue(a_section.c_str(), a_key.c_str(), static_cast<double>(a_value)) == SI_OK;
        } else if constexpr (std::is_same_v<T, long> || std::is_same_v<T, int>) {
            return a_ini.SetLongValue(a_section.c_str(), a_key.c_str(), static_cast<long>(a_value)) == SI_OK;
        } else if constexpr (std::is_same_v<T, std::string> || std::is_same_v<T, const char*>) {
            return a_ini.SetValue(a_section.c_str(), a_key.c_str(), a_value) == SI_OK;
        } else {
            static_assert(std::false_type::value, "SetValueInINI: Unsupported type for INI write");
            return false;
        }
    }

    bool CameraPathManager::ImportCameraPath(const char* a_filePath) {
        // Extract just the filename from the full path for GetValueFromINI
        std::filesystem::path fullPath(a_filePath);
        std::string filename = fullPath.filename().string();
        
        // Check if file exists
        if (!std::filesystem::exists(fullPath)) {
            log::error("FCSE - CameraPathManager: INI file does not exist: {}", a_filePath);
            return false;
        }
        
        // Clear existing path
        ClearPath(false);
        
        // Read General section
        bool useDegrees = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "UseDegrees:General", filename, true);
        float degToRad = useDegrees ? (PI / 180.0f) : 1.0f;
        
        log::info("FCSE - CameraPathManager: Importing path from {}, UseDegrees={}", a_filePath, useDegrees);
        
        // Parse file manually to handle duplicate section names properly
        std::ifstream file(a_filePath);
        if (!file.is_open()) {
            log::error("FCSE - CameraPathManager: Failed to open INI file: {}", a_filePath);
            return false;
        }
        
        std::string line;
        std::string currentSection;
        std::map<std::string, std::string> currentData;
        int translateCount = 0;
        int rotateCount = 0;
        
        auto processTranslatePoint = [&]() {
            if (currentData.find("Time") != currentData.end()) {
                double posX = currentData.count("PositionX") ? std::stod(currentData["PositionX"]) : 0.0;
                double posY = currentData.count("PositionY") ? std::stod(currentData["PositionY"]) : 0.0;
                double posZ = currentData.count("PositionZ") ? std::stod(currentData["PositionZ"]) : 0.0;
                double time = std::stod(currentData["Time"]);
                bool easeIn = currentData.count("EaseIn") ? (std::stoi(currentData["EaseIn"]) != 0) : true;
                bool easeOut = currentData.count("EaseOut") ? (std::stoi(currentData["EaseOut"]) != 0) : true;
                
                RE::NiPoint3 position(static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(posZ));
                Transition translationTrans(InterpolationType::kOn, static_cast<float>(time), easeIn, easeOut);
                CameraTranslationPoint point(translationTrans, position);
                AddTranslationPoint(point);
                
                translateCount++;
                log::info("FCSE - Imported TranslatePoint #{}: pos=({},{},{}), time={}, easeIn={}, easeOut={}",
                         translateCount, posX, posY, posZ, time, easeIn, easeOut);
            }
        };
        
        auto processRotatePoint = [&]() {
            if (currentData.find("Time") != currentData.end()) {
                double pitch = (currentData.count("Pitch") ? std::stod(currentData["Pitch"]) : 0.0) * degToRad;
                double yaw = (currentData.count("Yaw") ? std::stod(currentData["Yaw"]) : 0.0) * degToRad;
                double time = std::stod(currentData["Time"]);
                bool easeIn = currentData.count("EaseIn") ? (std::stoi(currentData["EaseIn"]) != 0) : true;
                bool easeOut = currentData.count("EaseOut") ? (std::stoi(currentData["EaseOut"]) != 0) : true;
                
                RE::BSTPoint2<float> rotation({static_cast<float>(pitch), static_cast<float>(yaw)});
                Transition rotationTrans(InterpolationType::kOn, static_cast<float>(time), easeIn, easeOut);
                CameraRotationPoint point(rotationTrans, rotation);
                AddRotationPoint(point);
                
                rotateCount++;
                log::info("FCSE - Imported RotatePoint #{}: pitch={}, yaw={}, time={}, easeIn={}, easeOut={}",
                         rotateCount, pitch, yaw, time, easeIn, easeOut);
            }
        };
        
        while (std::getline(file, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line.back() == ']') {
                // Process previous section if it was a point section
                if (currentSection == "TranslatePoint") {
                    processTranslatePoint();
                } else if (currentSection == "RotatePoint") {
                    processRotatePoint();
                }
                
                // Start new section
                currentSection = line.substr(1, line.length() - 2);
                currentData.clear();
                continue;
            }
            
            // Parse key=value pairs
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);
                
                // Trim key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // Remove comments from value
                size_t commentPos = value.find(';');
                if (commentPos != std::string::npos) {
                    value = value.substr(0, commentPos);
                    value.erase(value.find_last_not_of(" \t") + 1);
                }
                
                currentData[key] = value;
            }
        }
        
        // Process last section
        if (currentSection == "TranslatePoint") {
            processTranslatePoint();
        } else if (currentSection == "RotatePoint") {
            processRotatePoint();
        }
        
        file.close();
        
        log::info("FCSE - CameraPathManager: Loaded {} translation and {} rotation points from {}", 
                 m_translationPoints.size(), m_rotationPoints.size(), a_filePath);
        return true;
    }
    
    bool CameraPathManager::ExportCameraPath(const char* a_filePath) const {
        std::ofstream file(a_filePath);
        if (!file.is_open()) {
            log::error("FCSE - CameraPathManager: Failed to open INI file for writing: {}", a_filePath);
            return false;
        }
        
        // Write General section
        file << "[General]\n";
        file << "UseDegrees=1 ; Interpret rotation values as degrees (1) or radians (0)\n";
        file << "\n";
        
        float radToDeg = 180.0f / PI;
        
        // Write all translation points as duplicate [TranslatePoint] sections
        for (const auto& point : m_translationPoints) {
            file << "[TranslatePoint]\n";
            file << "PositionX=" << point.m_position.x << "\n";
            file << "PositionY=" << point.m_position.y << "\n";
            file << "PositionZ=" << point.m_position.z << "\n";
            file << "Time=" << point.m_transitionTranslate.m_time << "\n";
            file << "EaseIn=" << (point.m_transitionTranslate.m_easeIn ? 1 : 0) << "\n";
            file << "EaseOut=" << (point.m_transitionTranslate.m_easeOut ? 1 : 0) << "\n";
            file << "\n";
        }
        
        // Write all rotation points as duplicate [RotatePoint] sections
        for (const auto& point : m_rotationPoints) {
            file << "[RotatePoint]\n";
            file << "Pitch=" << (point.m_rotation.x * radToDeg) << "\n";
            file << "Yaw=" << (point.m_rotation.y * radToDeg) << "\n";
            file << "Time=" << point.m_transitionRotate.m_time << "\n";
            file << "EaseIn=" << (point.m_transitionRotate.m_easeIn ? 1 : 0) << "\n";
            file << "EaseOut=" << (point.m_transitionRotate.m_easeOut ? 1 : 0) << "\n";
            file << "\n";
        }
        
        file.close();
        
        if (!file.good()) {
            log::error("FCSE - CameraPathManager: Error occurred while writing INI file: {}", a_filePath);
            return false;
        }
        
        log::info("FCSE - CameraPathManager: Exported {} translation and {} rotation points to {}", 
                 m_translationPoints.size(), m_rotationPoints.size(), a_filePath);
        return true;
    }
    RE::NiPoint3 CameraPathManager::GetCurrentPosition() const {
        switch (m_interpolationMode) {
            case InterpolationMode::kNone:
                if (m_currentTranslationIndex < m_translationPoints.size()) {
                    return m_translationPoints[m_currentTranslationIndex].m_position;
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
                if (m_currentRotationIndex < m_rotationPoints.size()) {
                    return m_rotationPoints[m_currentRotationIndex].m_rotation;
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
        if (m_translationPoints.empty()) {
            return {0.f, 0.f, 0.f};
        }
        
        size_t currentIdx = m_currentTranslationIndex;
        if (currentIdx >= m_translationPoints.size()) {
            currentIdx = m_translationPoints.size() - 1;
        }
        
        const auto& currentPoint = m_translationPoints[currentIdx];
        const auto& currentPos = currentPoint.m_position;
        
        if (currentIdx == 0) {
            return currentPos;
        }
        
        const auto& prevPos = m_translationPoints[currentIdx - 1].m_position;
        
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionTranslateProgress,
                             currentPoint.m_transitionTranslate.m_easeIn,
                             currentPoint.m_transitionTranslate.m_easeOut);
        
        return prevPos + (currentPos - prevPos) * t;
    }
    
    RE::BSTPoint2<float> CameraPathManager::GetRotationLinear() const {
        if (m_rotationPoints.empty()) {
            return {0.f, 0.f};
        }
        
        size_t currentIdx = m_currentRotationIndex;
        if (currentIdx >= m_rotationPoints.size()) {
            currentIdx = m_rotationPoints.size() - 1;
        }
        
        const auto& currentPoint = m_rotationPoints[currentIdx];
        const auto& currentRot = currentPoint.m_rotation;
        
        if (currentIdx == 0) {
            return currentRot;
        }
        
        const auto& prevRot = m_rotationPoints[currentIdx - 1].m_rotation;
        
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

    // Helper: Compute Cubic Hermite basis functions
    static void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11) {
        float t2 = t * t;
        float t3 = t2 * t;
        
        h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;   // basis for p1
        h10 = t3 - 2.0f * t2 + t;              // basis for m1
        h01 = -2.0f * t3 + 3.0f * t2;          // basis for p2
        h11 = t3 - t2;                          // basis for m2
    }
    
    // Helper: Compute tangent for Cubic Hermite (scalar version for rotations)
    static float ComputeTangent(float v0, float v1, float v2, bool hasV0, bool hasV2) {
        if (hasV0 && hasV2) {
            return (v2 - v0) * 0.5f;  // Central difference
        } else if (!hasV0) {
            return v2 - v1;  // Forward difference (first segment)
        } else {
            return v2 - v1;  // Backward difference (last segment)
        }
    }
    
    // Helper: Check if two values are nearly identical
    static bool AreNearlyEqual(float a, float b, float epsilon = 0.001f) {
        return std::abs(a - b) < epsilon;
    }

    RE::NiPoint3 CameraPathManager::GetTranslationCubicHermite() const {
        if (m_translationPoints.empty()) {
            return {0.f, 0.f, 0.f};
        }
        
        if (m_translationPoints.size() == 1) {
            return m_translationPoints[0].m_position;
        }
        
        size_t currentIdx = m_currentTranslationIndex;
        if (currentIdx >= m_translationPoints.size()) {
            return m_translationPoints.back().m_position;
        }
        
        if (currentIdx == 0) {
            return m_translationPoints[0].m_position;
        }
        
        const auto& currentPoint = m_translationPoints[currentIdx];
        const auto& p1 = m_translationPoints[currentIdx - 1].m_position;
        const auto& p2 = currentPoint.m_position;
        
        // Check if points are identical
        float distSq = (p2.x - p1.x) * (p2.x - p1.x) + 
                       (p2.y - p1.y) * (p2.y - p1.y) + 
                       (p2.z - p1.z) * (p2.z - p1.z);
        if (distSq < 0.001f) {
            return p1;  // No interpolation needed
        }
        
        // Compute tangents
        bool hasP0 = currentIdx >= 2;
        bool hasP3 = currentIdx + 1 < m_translationPoints.size();
        
        const auto& p0 = hasP0 ? m_translationPoints[currentIdx - 2].m_position : p1;
        const auto& p3 = hasP3 ? m_translationPoints[currentIdx + 1].m_position : p2;
        
        RE::NiPoint3 m1, m2;
        m1.x = ComputeTangent(p0.x, p1.x, p2.x, hasP0, true);
        m1.y = ComputeTangent(p0.y, p1.y, p2.y, hasP0, true);
        m1.z = ComputeTangent(p0.z, p1.z, p2.z, hasP0, true);
        
        m2.x = ComputeTangent(p1.x, p2.x, p3.x, true, hasP3);
        m2.y = ComputeTangent(p1.y, p2.y, p3.y, true, hasP3);
        m2.z = ComputeTangent(p1.z, p2.z, p3.z, true, hasP3);
        
        // Apply easing and compute basis functions
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionTranslateProgress,
                             currentPoint.m_transitionTranslate.m_easeIn,
                             currentPoint.m_transitionTranslate.m_easeOut);
        
        float h00, h10, h01, h11;
        ComputeHermiteBasis(t, h00, h10, h01, h11);
        
        // Cubic Hermite interpolation
        RE::NiPoint3 result;
        result.x = h00 * p1.x + h10 * m1.x + h01 * p2.x + h11 * m2.x;
        result.y = h00 * p1.y + h10 * m1.y + h01 * p2.y + h11 * m2.y;
        result.z = h00 * p1.z + h10 * m1.z + h01 * p2.z + h11 * m2.z;
        
        return result;
    }
    
    RE::BSTPoint2<float> CameraPathManager::GetRotationCubicHermite() const {
        if (m_rotationPoints.empty()) {
            return {0.f, 0.f};
        }
        
        if (m_rotationPoints.size() == 1) {
            return m_rotationPoints[0].m_rotation;
        }
        
        size_t currentIdx = m_currentRotationIndex;
        if (currentIdx >= m_rotationPoints.size()) {
            return m_rotationPoints.back().m_rotation;
        }
        
        if (currentIdx == 0) {
            return m_rotationPoints[0].m_rotation;
        }
        
        const auto& currentPoint = m_rotationPoints[currentIdx];
        const auto& r1 = m_rotationPoints[currentIdx - 1].m_rotation;
        const auto& r2 = currentPoint.m_rotation;
        
        // Check if rotations are identical
        float pitchDiff = std::abs(_ts_SKSEFunctions::NormalRelativeAngle(r2.x - r1.x));
        float yawDiff = std::abs(_ts_SKSEFunctions::NormalRelativeAngle(r2.y - r1.y));
        if (AreNearlyEqual(pitchDiff, 0.0f) && AreNearlyEqual(yawDiff, 0.0f)) {
            return r1;  // No interpolation needed
        }
        
        // Normalize angles for interpolation
        float pitch1 = r1.x;
        float pitch2 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(r2.x - pitch1);
        float yaw1 = r1.y;
        float yaw2 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(r2.y - yaw1);
        
        // Get neighbor points with angle wrapping
        bool hasR0 = currentIdx >= 2;
        bool hasR3 = currentIdx + 1 < m_rotationPoints.size();
        
        float pitch0 = pitch1, yaw0 = yaw1;
        float pitch3 = pitch2, yaw3 = yaw2;
        
        if (hasR0) {
            const auto& r0 = m_rotationPoints[currentIdx - 2].m_rotation;
            pitch0 = pitch1 + _ts_SKSEFunctions::NormalRelativeAngle(r0.x - pitch1);
            yaw0 = yaw1 + _ts_SKSEFunctions::NormalRelativeAngle(r0.y - yaw1);
        }
        
        if (hasR3) {
            const auto& r3 = m_rotationPoints[currentIdx + 1].m_rotation;
            pitch3 = pitch2 + _ts_SKSEFunctions::NormalRelativeAngle(r3.x - r2.x);
            yaw3 = yaw2 + _ts_SKSEFunctions::NormalRelativeAngle(r3.y - r2.y);
        }
        
        // Compute tangents using helper function
        float m1_pitch = ComputeTangent(pitch0, pitch1, pitch2, hasR0, true);
        float m1_yaw = ComputeTangent(yaw0, yaw1, yaw2, hasR0, true);
        float m2_pitch = ComputeTangent(pitch1, pitch2, pitch3, true, hasR3);
        float m2_yaw = ComputeTangent(yaw1, yaw2, yaw3, true, hasR3);
        
        // Apply easing and compute basis functions
        float t = _ts_SKSEFunctions::ApplyEasing(m_transitionRotateProgress,
                             currentPoint.m_transitionRotate.m_easeIn,
                             currentPoint.m_transitionRotate.m_easeOut);
        
        float h00, h10, h01, h11;
        ComputeHermiteBasis(t, h00, h10, h01, h11);
        
        // Cubic Hermite interpolation
        float resultPitch = h00 * pitch1 + h10 * m1_pitch + h01 * pitch2 + h11 * m2_pitch;
        float resultYaw = h00 * yaw1 + h10 * m1_yaw + h01 * yaw2 + h11 * m2_yaw;
        
        RE::BSTPoint2<float> result;
        result.x = _ts_SKSEFunctions::NormalRelativeAngle(resultPitch);
        result.y = _ts_SKSEFunctions::NormalRelativeAngle(resultYaw);
        return result;
    }

} // namespace FCSE