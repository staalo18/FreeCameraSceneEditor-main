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
        
        size_t AddPoint(const PointType& a_point) { 
            size_t result = m_path.AddPoint(a_point);
            ResetTimeline();
            return result;
        }
        const PointType& GetPoint(size_t a_index) const { return m_path.GetPoint(a_index); }
        size_t EditPoint(size_t a_index, const PointType& a_point) { 
            size_t result = m_path.EditPoint(a_index, a_point);
            ResetTimeline();
            return result;
        }
        void RemovePoint(size_t a_index) { 
            m_path.RemovePoint(a_index);
            ResetTimeline();
        }
        void ClearTimeline() { 
            m_path.ClearPath();
            ResetTimeline();
        }
        size_t GetPointCount() const { return m_path.GetPointCount(); }
        
        bool AddTimelineFromFile(std::ifstream& a_file, float a_timeOffset = 0.0f, float a_conversionFactor = 1.0f) { 
            bool result = m_path.AddPathFromFile(a_file, a_timeOffset, a_conversionFactor);
            ResetTimeline();
            return result;
        }
        bool ExportTimeline(std::ofstream& a_file, float a_conversionFactor = 1.0f) const { 
            return m_path.ExportPath(a_file, a_conversionFactor);
        }
        PointType GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) {
            return m_path.GetPointAtCamera(a_time, a_easeIn, a_easeOut);
        }
        
        void UpdateTimeline(float a_currentTime) {
            size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return;
            }
            
            // Find the appropriate point index for current time
            size_t targetIndex = m_currentIndex;
            for (size_t i = m_currentIndex; i < pointCount; ++i) {
                const auto& point = GetPoint(i);
                if (a_currentTime <= point.m_transition.m_time) {
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
                m_progress = (a_currentTime - prevTime) / segmentDuration;
                m_progress = std::clamp(m_progress, 0.0f, 1.0f);
            } else {
                m_progress = 1.0f;
            }
        }
        
        // Get current interpolated value (returns the .m_point member of PointType)
        auto GetCurrentPoint() const -> decltype(std::declval<PointType>().m_point) {
            if (GetPointCount() == 0) {
                return PointType{}.m_point;
            }
            
            size_t currentIdx = m_currentIndex;
            if (currentIdx >= GetPointCount()) {
                currentIdx = GetPointCount() - 1;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            
            switch (currentPoint.m_transition.m_mode) {
                case InterpolationMode::kNone:
                    return currentPoint.m_point;
                case InterpolationMode::kLinear:
                    return GetPointLinear();
                case InterpolationMode::kCubicHermite:
                    return GetPointCubicHermite();
                default:
                    return GetPointLinear();
            }
        }
        
        size_t GetCurrentIndex() const { return m_currentIndex; }
        float GetProgress() const { return m_progress; }
        bool IsComplete() const { return m_currentIndex >= GetPointCount(); }
        
        void ResetTimeline() {
            m_currentIndex = 0;
            m_progress = 0.0f;
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
            
            float t = _ts_SKSEFunctions::ApplyEasing(m_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            PointType result = pt1.CubicHermite(pt0, pt1, pt2, pt3, t);

            return result.m_point;
        }
        
        PathType m_path;  //TranslationPath or RotationPath
        
        size_t m_currentIndex = 0;
        float m_progress = 0.0f;  // 0.0 to 1.0 within current segment
    };
    
    using TranslationTimeline = Timeline<TranslationPath>;
    using RotationTimeline = Timeline<RotationPath>;
} // namespace FCSE

