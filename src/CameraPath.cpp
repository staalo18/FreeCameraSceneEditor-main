#include "CameraPath.h"
#include "FCSE_Utils.h"


namespace FCSE {
    
    // ===== TranslationPath implementations =====

    TranslationPoint TranslationPath::GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) {
        TranslationPoint point;
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        RE::FreeCameraState* cameraState = nullptr;
        
        if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
            
            if (cameraState) {
                point.m_transition = Transition(InterpolationType::kOn, a_time, a_easeIn, a_easeOut);
                point.m_point = cameraState->translation;
            }
        }  

        return point;
    }

    bool TranslationPath::ImportPath(std::ifstream& a_file, float a_conversionFactor) {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }
        
        ClearPath();
        
        return ParseFCSETimelineFileSections(a_file, "TranslatePoint", [this](const auto& data) {
            if (data.find("Time") != data.end()) {
                float posX = data.count("PositionX") ? std::stof(data.at("PositionX")) : 0.0f;
                float posY = data.count("PositionY") ? std::stof(data.at("PositionY")) : 0.0f;
                float posZ = data.count("PositionZ") ? std::stof(data.at("PositionZ")) : 0.0f;
                float time = std::stof(data.at("Time"));
                bool easeIn = data.count("EaseIn") ? (std::stoi(data.at("EaseIn")) != 0) : true;
                bool easeOut = data.count("EaseOut") ? (std::stoi(data.at("EaseOut")) != 0) : true;
                
                RE::NiPoint3 position(posX, posY, posZ);
                Transition translationTrans(InterpolationType::kOn, time, easeIn, easeOut);
                TranslationPoint point(translationTrans, position);
                AddPoint(point);
            }
        });
    }

    bool TranslationPath::ExportPath(std::ofstream& a_file, float a_conversionFactor) const {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }

        for (const auto& point : m_points) {
            a_file << "[TranslatePoint]\n";
            a_file << "PositionX=" << point.m_point.x << "\n";
            a_file << "PositionY=" << point.m_point.y << "\n";
            a_file << "PositionZ=" << point.m_point.z << "\n";
            a_file << "Time=" << point.m_transition.m_time << "\n";
            a_file << "EaseIn=" << (point.m_transition.m_easeIn ? 1 : 0) << "\n";
            a_file << "EaseOut=" << (point.m_transition.m_easeOut ? 1 : 0) << "\n";
            a_file << "\n";
            
            if (!a_file.good()) {
                log::error("{}: Write error", __FUNCTION__);
                return false;
            }
        }
        
        return true;
    }

    // ===== RotationPath implementations =====
    
    RotationPoint RotationPath::GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) {
        RotationPoint point;
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        RE::FreeCameraState* cameraState = nullptr;
        
        if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
            
            if (cameraState) {
                point.m_transition = Transition(InterpolationType::kOn, a_time, a_easeIn, a_easeOut);
                point.m_point = cameraState->rotation;
            }
        }  

        return point;
    }

    bool RotationPath::ImportPath(std::ifstream& a_file, float a_conversionFactor) {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }
        
        ClearPath();
        
        return ParseFCSETimelineFileSections(a_file, "RotatePoint", [this, a_conversionFactor](const auto& data) {
            if (data.find("Time") != data.end()) {
                float pitch = (data.count("Pitch") ? std::stof(data.at("Pitch")) : 0.0f) * a_conversionFactor;
                float yaw = (data.count("Yaw") ? std::stof(data.at("Yaw")) : 0.0f) * a_conversionFactor;
                float time = std::stof(data.at("Time"));
                bool easeIn = data.count("EaseIn") ? (std::stoi(data.at("EaseIn")) != 0) : true;
                bool easeOut = data.count("EaseOut") ? (std::stoi(data.at("EaseOut")) != 0) : true;
                
                RE::BSTPoint2<float> rotation({pitch, yaw});
                Transition rotationTrans(InterpolationType::kOn, time, easeIn, easeOut);
                RotationPoint point(rotationTrans, rotation);
                AddPoint(point);
            }
        });
    }

    bool RotationPath::ExportPath(std::ofstream& a_file, float a_conversionFactor) const {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }
     
        for (const auto& point : m_points) {
            a_file << "[RotatePoint]\n";
            a_file << "Pitch=" << (point.m_point.x * a_conversionFactor) << "\n";
            a_file << "Yaw=" << (point.m_point.y * a_conversionFactor) << "\n";
            a_file << "Time=" << point.m_transition.m_time << "\n";
            a_file << "EaseIn=" << (point.m_transition.m_easeIn ? 1 : 0) << "\n";
            a_file << "EaseOut=" << (point.m_transition.m_easeOut ? 1 : 0) << "\n";
            a_file << "\n";
            
            if (!a_file.good()) {
                log::error("{}: Write error", __FUNCTION__);
                return false;
            }
        }

        return true;
    }

} // namespace FCSE
