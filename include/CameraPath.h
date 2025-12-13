#pragma once

#include "CameraTypes.h"
#include "_ts_SKSEFunctions.h"
#include "FCSE_Utils.h"
#include <stdexcept>


namespace FCSE {
    class TranslationPoint {
    public:
        TranslationPoint()
            : m_transition(InterpolationType::kInvalid, 0.0f, false, false)
            , m_point({0.f, 0.f, 0.f}) {}

        TranslationPoint(const Transition& a_transition, const RE::NiPoint3& a_point)
            : m_transition(a_transition)
            , m_point(a_point) {}
       
        bool IsNearlyEqual(const TranslationPoint& other) const {
            return std::abs(m_point.x - other.m_point.x) < EPSILON_COMPARISON &&
                   std::abs(m_point.y - other.m_point.y) < EPSILON_COMPARISON &&
                   std::abs(m_point.z - other.m_point.z) < EPSILON_COMPARISON;
        }
       
        void Wrap() {} // Do nothing for translation
        
        TranslationPoint UnWrap(const TranslationPoint&) const {
            return *this; // No unwrapping needed for translation
        }
       
        TranslationPoint operator+(const TranslationPoint& other) const {
            return TranslationPoint(m_transition, m_point + other.m_point);
        }
        
        TranslationPoint operator-(const TranslationPoint& other) const {
            return TranslationPoint(m_transition, m_point - other.m_point);
        }
        
        TranslationPoint operator*(float scalar) const {
            return TranslationPoint(m_transition, m_point * scalar);
        }

        Transition m_transition;
        RE::NiPoint3 m_point;
    };

    class RotationPoint {
    public:
        RotationPoint()
            : m_transition(InterpolationType::kInvalid, 0.0f, false, false)
            , m_point({0.f, 0.f}) {}

        RotationPoint(const Transition& a_transition, const RE::BSTPoint2<float>& a_point)
            : m_transition(a_transition)
            , m_point(a_point) {}
       
        bool IsNearlyEqual(const RotationPoint& other) const {
            return std::abs(m_point.x - other.m_point.x) < EPSILON_COMPARISON &&
                   std::abs(m_point.y - other.m_point.y) < EPSILON_COMPARISON;
        }

        void Wrap() {
            m_point.x = _ts_SKSEFunctions::NormalRelativeAngle(m_point.x);
            m_point.y = _ts_SKSEFunctions::NormalRelativeAngle(m_point.y);
        }

        RotationPoint UnWrap(const RotationPoint& reference) const {
            RotationPoint diff = *this - reference;
            diff.m_point.x = _ts_SKSEFunctions::NormalRelativeAngle(diff.m_point.x);
            diff.m_point.y = _ts_SKSEFunctions::NormalRelativeAngle(diff.m_point.y);
            RotationPoint result = reference + diff;
            return result;
        }

        // Raw arithmetic operators - DO NOT wrap (needed for unwrapped space calculations)
        RotationPoint operator+(const RotationPoint& other) const {
            RE::BSTPoint2<float> result;
            result.x = m_point.x + other.m_point.x;
            result.y = m_point.y + other.m_point.y;
            return RotationPoint(m_transition, result);
        }
        
        RotationPoint operator-(const RotationPoint& other) const {
            RE::BSTPoint2<float> result;
            result.x = m_point.x - other.m_point.x;
            result.y = m_point.y - other.m_point.y;
            return RotationPoint(m_transition, result);
        }
        
        RotationPoint operator*(float scalar) const {
            RE::BSTPoint2<float> result;
            result.x = m_point.x * scalar;
            result.y = m_point.y * scalar;
            return RotationPoint(m_transition, result);
        }
       
        Transition m_transition;
        RE::BSTPoint2<float> m_point;
    };

    template<typename PointType>
    class CameraPath {
    public:
        virtual ~CameraPath() = default;
                
        size_t AddPoint(const PointType& a_point) {
            PointType modifiedPoint = a_point;
            if (modifiedPoint.m_transition.m_time < 0.0f) {
                modifiedPoint.m_transition.m_time = 0.0f;
            } 

            // Insert point in sorted order by time
            auto insertPos = std::lower_bound(m_points.begin(), m_points.end(), modifiedPoint.m_transition.m_time,
                [](const PointType& point, float time) {
                    return point.m_transition.m_time < time;
                });
            
            auto it = m_points.insert(insertPos, modifiedPoint);
            return std::distance(m_points.begin(), it);
        }
        
        const PointType& GetPoint(size_t a_index) const {
            if (a_index >= m_points.size()) {
                log::error("{}: index out of range", __FUNCTION__);
                throw std::out_of_range("CameraPath::GetPoint: index out of range");
            }
            return m_points[a_index];
        }
        
        size_t EditPoint(size_t a_index, const PointType& a_point) {
            if (a_index >= m_points.size()) {
                log::error("{}: index out of range", __FUNCTION__);
                throw std::out_of_range("CameraPath::EditPoint: index out of range");
            }
            
            // If time hasn't changed, simple update
            if (std::abs(m_points[a_index].m_transition.m_time - a_point.m_transition.m_time) < EPSILON_COMPARISON) {
                m_points[a_index] = a_point;
                return a_index;
            }
            
            // Time changed: remove old point and re-insert to maintain sorted order
            m_points.erase(m_points.begin() + a_index);
                        
            return AddPoint(a_point);
        }
        
        void RemovePoint(size_t a_index) {
            if (a_index < m_points.size()) {
                m_points.erase(m_points.begin() + a_index);
            }
        }
        
        void ClearPath() {
            m_points.clear();
        }
        
        size_t GetPointCount() const { return m_points.size(); }

        virtual bool ImportPath(std::ifstream& a_file, float a_conversionFactor = 1.0f) = 0;
        virtual bool ExportPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const = 0;
        virtual PointType GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) = 0;
        
    protected:
        std::vector<PointType> m_points;
    };

    class TranslationPath : public CameraPath<TranslationPoint> {
    public:
        using PointType = TranslationPoint;
        
        TranslationPoint GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) override;
        bool ImportPath(std::ifstream& a_file, float a_conversionFactor = 1.0f) override;
        bool ExportPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const override;
    };

    class RotationPath : public CameraPath<RotationPoint> {
    public:
        using PointType = RotationPoint;
        
        RotationPoint GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) override;
        bool ImportPath(std::ifstream& a_file, float a_conversionFactor = 1.0f) override;
        bool ExportPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const override;
    };

} // namespace FCSE