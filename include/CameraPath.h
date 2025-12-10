#pragma once

#include "CameraTypes.h"
#include "_ts_SKSEFunctions.h"


namespace FCSE {
    class CameraPoint {
    public:
        CameraPoint() 
            : m_transition(InterpolationType::kInvalid, 0.0f, false, false) {}
        
        CameraPoint(const Transition& a_transition)
            : m_transition(a_transition) {}
        
        virtual ~CameraPoint() = default;
        
        // Virtual method for comparing points - must be overridden by derived classes
        virtual bool IsNearlyEqual(const CameraPoint& other, float epsilon = 0.0001f) const = 0;

        virtual void Wrap() = 0; // Wrap angles if necessary
        
        Transition m_transition;
    };

    class TranslationPoint : public CameraPoint {
    public:
        TranslationPoint()
            : CameraPoint()
            , m_point({0.f, 0.f, 0.f}) {}

        TranslationPoint(const Transition& a_transition, const RE::NiPoint3& a_point)
            : CameraPoint(a_transition)
            , m_point(a_point) {}
       
        bool IsNearlyEqual(const CameraPoint& other, float epsilon = 0.0001f) const override {
            const auto* otherPoint = dynamic_cast<const TranslationPoint*>(&other);
            if (!otherPoint) return false;
            
            return std::abs(m_point.x - otherPoint->m_point.x) < epsilon &&
                   std::abs(m_point.y - otherPoint->m_point.y) < epsilon &&
                   std::abs(m_point.z - otherPoint->m_point.z) < epsilon;
        }
       
        void Wrap() override {
            ; // Do nothing for translation
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

        RE::NiPoint3 m_point;
    };

    class RotationPoint : public CameraPoint {
    public:
        RotationPoint()
            : CameraPoint()
            , m_point({0.f, 0.f}) {}

        RotationPoint(const Transition& a_transition, const RE::BSTPoint2<float>& a_point)
            : CameraPoint(a_transition)
            , m_point(a_point) {}
       
        bool IsNearlyEqual(const CameraPoint& other, float epsilon = 0.0001f) const override {
            const auto* otherPoint = dynamic_cast<const RotationPoint*>(&other);
            if (!otherPoint) return false;
            
            return std::abs(m_point.x - otherPoint->m_point.x) < epsilon &&
                   std::abs(m_point.y - otherPoint->m_point.y) < epsilon;
        }

        void Wrap() override {
            m_point.x = _ts_SKSEFunctions::NormalRelativeAngle(m_point.x);
            m_point.y = _ts_SKSEFunctions::NormalRelativeAngle(m_point.y);
        }

        RotationPoint operator+(const RotationPoint& other) const {
            RE::BSTPoint2<float> result;
            result.x = _ts_SKSEFunctions::NormalRelativeAngle(m_point.x + other.m_point.x);
            result.y = _ts_SKSEFunctions::NormalRelativeAngle(m_point.y + other.m_point.y);
            return RotationPoint(m_transition, result);
        }
        
        RotationPoint operator-(const RotationPoint& other) const {
            RE::BSTPoint2<float> result;
            result.x = _ts_SKSEFunctions::NormalRelativeAngle(m_point.x - other.m_point.x);
            result.y = _ts_SKSEFunctions::NormalRelativeAngle(m_point.y - other.m_point.y);
            return RotationPoint(m_transition, result);
        }
        
        RotationPoint operator*(float scalar) const {
            RE::BSTPoint2<float> result;
            result.x = _ts_SKSEFunctions::NormalRelativeAngle(m_point.x * scalar);
            result.y = _ts_SKSEFunctions::NormalRelativeAngle(m_point.y * scalar);
            return RotationPoint(m_transition, result);
        }
       
        RE::BSTPoint2<float> m_point;
    };

    template<typename PointType>
    class CameraPath {
    public:
        virtual ~CameraPath() = default;
                
        void AddPoint(PointType a_point) {
            // Validate and clamp time value to non-negative
            if (a_point.m_transition.m_time < 0.0f) {
                a_point.m_transition.m_time = 0.0f;
            }
            
            // Insert point in sorted order by time
            float pointTime = a_point.m_transition.m_time;
            auto insertPos = std::lower_bound(m_points.begin(), m_points.end(), pointTime,
                [](const PointType& point, float time) {
                    return point.m_transition.m_time < time;
                });
            
            m_points.insert(insertPos, a_point);
        }
        
        const PointType& GetPoint(size_t a_index) const {
            return m_points[a_index];
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
        virtual PointType GetCurrentPoint(float a_time, bool a_easeIn, bool a_easeOut) = 0;
        
    protected:
        std::vector<PointType> m_points;
    };

    class TranslationPath : public CameraPath<TranslationPoint> {
    public:
        using PointType = TranslationPoint;
        
        TranslationPoint GetCurrentPoint(float a_time, bool a_easeIn, bool a_easeOut) override;
        bool ImportPath(std::ifstream& a_file, float a_conversionFactor = 1.0f) override;
        bool ExportPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const override;
    };

    class RotationPath : public CameraPath<RotationPoint> {
    public:
        using PointType = RotationPoint;
        
        RotationPoint GetCurrentPoint(float a_time, bool a_easeIn, bool a_easeOut) override;
        bool ImportPath(std::ifstream& a_file, float a_conversionFactor = 1.0f) override;
        bool ExportPath(std::ofstream& a_file, float a_conversionFactor = 1.0f) const override;
    };

} // namespace FCSE