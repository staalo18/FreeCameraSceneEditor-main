#include "CameraPath.h"
#include "FCSE_Utils.h"


namespace FCSE {
    
    // ===== TranslationPath implementations =====

    TranslationPoint TranslationPath::GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) {
        TranslationPoint point;
        point.m_transition = Transition(a_time, InterpolationMode::kCubicHermite, a_easeIn, a_easeOut);
        point.m_point = GetFreeCameraTranslation();

        return point;
    }

    bool TranslationPath::AddPathFromFile(std::ifstream& a_file, float a_timeOffset, float a_conversionFactor) {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }
        
        return ParseFCSETimelineFileSections(a_file, "TranslatePoint", [this, a_timeOffset](const auto& data) {
            if (data.find("Time") != data.end()) {
                try {
                    float time = std::stof(data.at("Time")) + a_timeOffset;
                    bool easeIn = data.count("EaseIn") ? (std::stoi(data.at("EaseIn")) != 0) : false;
                    bool easeOut = data.count("EaseOut") ? (std::stoi(data.at("EaseOut")) != 0) : false;
                    
                    int modeValue = data.count("InterpolationMode") ? std::stoi(data.at("InterpolationMode")) : 2;
                    InterpolationMode mode = ToInterpolationMode(modeValue);
                    
                    Transition translationTrans(time, mode, easeIn, easeOut);
                    
                    // Check if this is a reference-based point
                    if (data.count("UseRef") && std::stoi(data.at("UseRef")) != 0) {
                        // Reference-based point
                        uint32_t formID = data.count("RefFormID") ? std::stoul(data.at("RefFormID"), nullptr, 16) : 0;
                        float offsetX = data.count("OffsetX") ? std::stof(data.at("OffsetX")) : 0.0f;
                        float offsetY = data.count("OffsetY") ? std::stof(data.at("OffsetY")) : 0.0f;
                        float offsetZ = data.count("OffsetZ") ? std::stof(data.at("OffsetZ")) : 0.0f;
                        bool isOffsetRelative = data.count("isOffsetRelative") ? (std::stoi(data.at("isOffsetRelative")) != 0) : false;
                    
                    RE::TESObjectREFR* reference = nullptr;
                    if (formID != 0) {
                        auto* form = RE::TESForm::LookupByID(formID);
                        reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                    }
                    
                    if (reference) {
                        RE::NiPoint3 offset(offsetX, offsetY, offsetZ);
                        TranslationPoint point(translationTrans, reference, offset, isOffsetRelative);
                        AddPoint(point);
                    } else {
                        log::warn("{}: Failed to resolve reference FormID 0x{:X}, using offset as absolute position", __FUNCTION__, formID);
                        RE::NiPoint3 position(offsetX, offsetY, offsetZ);
                        TranslationPoint point(translationTrans, position);
                        AddPoint(point);
                    }
                } else {
                    // Static position-based point
                    float posX = data.count("PositionX") ? std::stof(data.at("PositionX")) : 0.0f;
                    float posY = data.count("PositionY") ? std::stof(data.at("PositionY")) : 0.0f;
                    float posZ = data.count("PositionZ") ? std::stof(data.at("PositionZ")) : 0.0f;
                    
                    RE::NiPoint3 position(posX, posY, posZ);
                    TranslationPoint point(translationTrans, position);
                    AddPoint(point);
                }
                } catch (const std::exception& e) {
                    log::warn("{}: Skipping invalid TranslatePoint entry: {}", __FUNCTION__, e.what());
                }
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
            
            if (point.m_useRef && point.m_reference) {
                // Reference-based point
                a_file << "UseRef=1\n";
                a_file << "RefFormID=0x" << std::hex << std::uppercase << point.m_reference->GetFormID() << std::dec << "\n";
                a_file << "OffsetX=" << point.m_offset.x << "\n";
                a_file << "OffsetY=" << point.m_offset.y << "\n";
                a_file << "OffsetZ=" << point.m_offset.z << "\n";
                a_file << "isOffsetRelative=" << (point.m_isOffsetRelative ? 1 : 0) << "\n";
            } else {
                // Static position-based point
                a_file << "UseRef=0\n";
                a_file << "PositionX=" << point.m_point.x << "\n";
                a_file << "PositionY=" << point.m_point.y << "\n";
                a_file << "PositionZ=" << point.m_point.z << "\n";
            }
            
            a_file << "Time=" << point.m_transition.m_time << "\n";
            a_file << "InterpolationMode=" << static_cast<int>(point.m_transition.m_mode) << "\n";
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
    
    RotationPoint RotationPath::GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) {
        RotationPoint point;
        auto* playerCamera = RE::PlayerCamera::GetSingleton();
        RE::FreeCameraState* cameraState = nullptr;
        
        if (playerCamera && playerCamera->currentState && (playerCamera->currentState->id == RE::CameraState::kFree)) {
            cameraState = static_cast<RE::FreeCameraState*>(playerCamera->currentState.get());
            
            if (cameraState) {
                point.m_transition = Transition(a_time, InterpolationMode::kCubicHermite, a_easeIn, a_easeOut);
                point.m_point = cameraState->rotation;
            }
        }  

        return point;
    }

    bool RotationPath::AddPathFromFile(std::ifstream& a_file, float a_timeOffset, float a_conversionFactor) {
        if (!a_file.is_open()) {
            log::error("{}: File is not open", __FUNCTION__);
            return false;
        }
        
        return ParseFCSETimelineFileSections(a_file, "RotatePoint", [this, a_conversionFactor, a_timeOffset](const auto& data) {
            if (data.find("Time") != data.end()) {
                try {
                    float time = std::stof(data.at("Time")) + a_timeOffset;
                    bool easeIn = data.count("EaseIn") ? (std::stoi(data.at("EaseIn")) != 0) : false;
                    bool easeOut = data.count("EaseOut") ? (std::stoi(data.at("EaseOut")) != 0) : false;
                    
                    int modeValue = data.count("InterpolationMode") ? std::stoi(data.at("InterpolationMode")) : 2;
                    InterpolationMode mode =ToInterpolationMode(modeValue);
                    
                    Transition rotationTrans(time, mode, easeIn, easeOut);
                    
                    // Check if this is a reference-based point
                    if (data.count("UseRef") && std::stoi(data.at("UseRef")) != 0) {
                        // Reference-based point
                        uint32_t formID = data.count("RefFormID") ? std::stoul(data.at("RefFormID"), nullptr, 16) : 0;
                        float offsetPitch = (data.count("OffsetPitch") ? std::stof(data.at("OffsetPitch")) : 0.0f) * a_conversionFactor;
                        float offsetYaw = (data.count("OffsetYaw") ? std::stof(data.at("OffsetYaw")) : 0.0f) * a_conversionFactor;
                        
                        RE::TESObjectREFR* reference = nullptr;
                        if (formID != 0) {
                            auto* form = RE::TESForm::LookupByID(formID);
                            reference = form ? form->As<RE::TESObjectREFR>() : nullptr;
                        }
                        
                        if (reference) {
                            RE::BSTPoint2<float> offset({offsetPitch, offsetYaw});
                            RotationPoint point(rotationTrans, reference, offset);
                            AddPoint(point);
                        } else {
                            log::warn("{}: Failed to resolve reference FormID 0x{:X}, using offset as absolute rotation", __FUNCTION__, formID);
                            RE::BSTPoint2<float> rotation({offsetPitch, offsetYaw});
                            RotationPoint point(rotationTrans, rotation);
                            AddPoint(point);
                        }
                    } else {
                        // Static rotation-based point
                        float pitch = (data.count("Pitch") ? std::stof(data.at("Pitch")) : 0.0f) * a_conversionFactor;
                        float yaw = (data.count("Yaw") ? std::stof(data.at("Yaw")) : 0.0f) * a_conversionFactor;
                        
                        RE::BSTPoint2<float> rotation({pitch, yaw});
                        RotationPoint point(rotationTrans, rotation);
                        AddPoint(point);
                    }
                } catch (const std::exception& e) {
                    log::warn("{}: Skipping invalid RotatePoint entry: {}", __FUNCTION__, e.what());
                }
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
            
            if (point.m_useRef && point.m_reference) {
                // Reference-based point
                a_file << "UseRef=1\n";
                a_file << "RefFormID=0x" << std::hex << std::uppercase << point.m_reference->GetFormID() << std::dec << "\n";
                a_file << "OffsetPitch=" << (point.m_offset.x * a_conversionFactor) << "\n";
                a_file << "OffsetYaw=" << (point.m_offset.y * a_conversionFactor) << "\n";
            } else {
                // Static rotation-based point
                a_file << "UseRef=0\n";
                a_file << "Pitch=" << (point.m_point.x * a_conversionFactor) << "\n";
                a_file << "Yaw=" << (point.m_point.y * a_conversionFactor) << "\n";
            }
            
            a_file << "Time=" << point.m_transition.m_time << "\n";
            a_file << "InterpolationMode=" << static_cast<int>(point.m_transition.m_mode) << "\n";
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
