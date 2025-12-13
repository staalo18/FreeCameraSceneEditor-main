#pragma once

#include "CameraPath.h"
#include "FCSE_Utils.h"

namespace FCSE {
    // Template class for timeline traversal and interpolation
    template<typename PathType>
    class Timeline {
    public:
        using PointType = typename PathType::PointType;
        
        Timeline() = default;
        ~Timeline() = default;
        
        size_t AddPoint(const PointType& a_point) { return m_path.AddPoint(a_point); }
        const PointType& GetPoint(size_t a_index) const { return m_path.GetPoint(a_index); }
        size_t EditPoint(size_t a_index, const PointType& a_point) { return m_path.EditPoint(a_index, a_point); }
        void RemovePoint(size_t a_index) { m_path.RemovePoint(a_index); }
        void ClearTimeline() { 
            m_path.ClearPath();
            ResetTimeline();
        }
        size_t GetPointCount() const { return m_path.GetPointCount(); }
        
        bool ImportTimeline(std::ifstream& a_file, float a_conversionFactor = 1.0f) { 
            bool result = m_path.ImportPath(a_file, a_conversionFactor);
            ResetTimeline();
            return result;
        }
        bool ExportTimeline(std::ofstream& a_file, float a_conversionFactor = 1.0f) const { 
            return m_path.ExportPath(a_file, a_conversionFactor);
        }
        PointType GetPointAtCameraPos(float a_time, bool a_easeIn, bool a_easeOut) {
            return m_path.GetPointAtCameraPos(a_time, a_easeIn, a_easeOut);
        }
        
        void UpdateTimeline(float a_currentTime) {
            m_currentTime = a_currentTime;
            size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return;
            }
            
            // Find the appropriate point index for current time
            size_t targetIndex = m_currentIndex;
            for (size_t i = m_currentIndex; i < pointCount; ++i) {
                const auto& point = GetPoint(i);
                if (m_currentTime <= point.m_transition.m_time) {
                    targetIndex = i;
                    break;
                }
                targetIndex = i + 1;
            }
            
            if (targetIndex >= pointCount) {
                m_currentIndex = pointCount;
                m_progress = 1.0f;
                return;
            }
            
            m_currentIndex = targetIndex;
            const auto& currentPoint = GetPoint(targetIndex);
            
            // Calculate progress within current segment
            float prevTime = 0.0f;
            if (targetIndex > 0) {
                const auto& prevPoint = GetPoint(targetIndex - 1);
                prevTime = prevPoint.m_transition.m_time;
            }
            
            float segmentDuration = currentPoint.m_transition.m_time - prevTime;
            if (segmentDuration > 0.0f) {
                m_progress = (m_currentTime - prevTime) / segmentDuration;
                m_progress = std::clamp(m_progress, 0.0f, 1.0f);
            } else {
                m_progress = 1.0f;
            }
        }
        
        // Get current interpolated value (returns the .m_point member of PointType)
        auto GetCurrentPoint() const -> decltype(std::declval<PointType>().m_point) {
            switch (m_interpolationMode) {
                case InterpolationMode::kNone:
                    if (m_currentIndex < GetPointCount()) {
                        return GetPoint(m_currentIndex).m_point;
                    }
                    return PointType{}.m_point;
                case InterpolationMode::kLinear:
                    return GetPointLinear();
                case InterpolationMode::kCubicHermite:
                    return GetPointCubicHermite();
                default:
                    return GetPointLinear();
            }
        }
        
        void SetInterpolationMode(InterpolationMode a_mode) { m_interpolationMode = a_mode; }
        InterpolationMode GetInterpolationMode() const { return m_interpolationMode; }
        
        size_t GetCurrentIndex() const { return m_currentIndex; }
        float GetProgress() const { return m_progress; }
        float GetCurrentTime() const { return m_currentTime; }
        bool IsComplete() const { return m_currentIndex >= GetPointCount(); }
        
        void ResetTimeline() {
            m_currentIndex = 0;
            m_progress = 0.0f;
            m_currentTime = 0.0f;
        }
        
    private:
        auto GetPointLinear() const -> decltype(std::declval<PointType>().m_point) {
            if (GetPointCount() == 0) {
                return PointType{}.m_point;
            }
            
            size_t currentIdx = m_currentIndex;
            if (currentIdx >= GetPointCount()) {
                currentIdx = GetPointCount() - 1;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            
            if (currentIdx == 0) {
                return currentPoint.m_point;
            }
            
            const auto& prevPoint = GetPoint(currentIdx - 1);
            
            if (prevPoint.IsNearlyEqual(currentPoint)) {
                return currentPoint.m_point;
            }
            
            float t = _ts_SKSEFunctions::ApplyEasing(m_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            // Linear interpolation using PointType operators
            PointType result = prevPoint + (currentPoint - prevPoint) * t;
            return result.m_point;
        }
        
        auto GetPointCubicHermite() const -> decltype(std::declval<PointType>().m_point) {
            const size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return PointType{}.m_point;
            }
            
            if (pointCount == 1) {
                return GetPoint(0).m_point;
            }
            
            size_t currentIdx = m_currentIndex;
            if (currentIdx >= pointCount) {
                return GetPoint(pointCount - 1).m_point;
            }
            
            if (currentIdx == 0) {
                return GetPoint(0).m_point;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            const auto& prevPoint = GetPoint(currentIdx - 1);
            
            if (prevPoint.IsNearlyEqual(currentPoint)) {
                return prevPoint.m_point;
            }
            
            // Get neighboring points for tangent computation
            bool hasP0 = currentIdx >= 2;
            bool hasP3 = currentIdx + 1 < GetPointCount();
            
            PointType pt0 = hasP0 ? GetPoint(currentIdx - 2) : prevPoint;
            PointType pt1 = prevPoint;
            PointType pt2 = currentPoint;
            PointType pt3 = hasP3 ? GetPoint(currentIdx + 1) : currentPoint;
            
            // Wrap angles for rotation interpolation (no-op for translation)
            pt0.Wrap();
            pt1.Wrap();
            pt2.Wrap();
            pt3.Wrap();
            
            // Compute tangent vectors using PointType operators
            PointType m1 = hasP0 ? (pt2 - pt0) * 0.5f : (pt2 - pt1);
            PointType m2 = hasP3 ? (pt3 - pt1) * 0.5f : (pt2 - pt1);
            
            float t = _ts_SKSEFunctions::ApplyEasing(m_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            float h00, h10, h01, h11;
            ComputeHermiteBasis(t, h00, h10, h01, h11);
            
            // Cubic Hermite interpolation using vector operators
            PointType result = pt1 * h00 + m1 * h10 + pt2 * h01 + m2 * h11;
            result.Wrap();  // Wrap final result for rotations
            
            return result.m_point;
        }
        
        PathType m_path;  //TranslationPath or RotationPath
        
        InterpolationMode m_interpolationMode = InterpolationMode::kCubicHermite;
        size_t m_currentIndex = 0;
        float m_progress = 0.0f;  // 0.0 to 1.0 within current segment
        float m_currentTime = 0.0f;
    };
    
    using TranslationTimeline = Timeline<TranslationPath>;
    using RotationTimeline = Timeline<RotationPath>;
} // namespace FCSE

