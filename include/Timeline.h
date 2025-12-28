#pragma once

#include "CameraPath.h"
#include "FCSE_Utils.h"

namespace FCSE {
    enum class PlaybackMode : int {
        kEnd = 0,   // Stop at end of timeline (default)
        kLoop = 1   // Restart from beginning when timeline completes
    };

    // Template class for timeline playback and interpolation
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
        
        void UpdateTimeline(float a_deltaTime) {
            if (m_isPaused || !m_isPlaying) {
                return;
            }

            size_t pointCount = GetPointCount();
            if (pointCount == 0) {
                m_isPlaying = false;
                return;
            }
            
            m_currentTime += a_deltaTime;
            
            // Get timeline duration (includes loop offset if in loop mode)
            float timelineDuration = GetTimelineDuration();
            
            // Check for completion and handle loop
            if (m_currentTime >= timelineDuration) {
                if (m_playbackMode == PlaybackMode::kLoop) {
                    // Loop: use modulo to wrap time seamlessly
                    m_currentTime = std::fmod(m_currentTime, timelineDuration);
                } else {
                    // End: clamp to final position and stop
                    m_currentTime = timelineDuration;
                    m_isPlaying = false;
                }
            }
            
            float lastPointTime = GetPoint(pointCount - 1).m_transition.m_time;
            
            // In loop mode with offset, check if we're in the virtual last segment
            if (m_playbackMode == PlaybackMode::kLoop && m_loopTimeOffset > 0.0f && m_currentTime > lastPointTime) {
                // We're in the loop offset segment (last point -> first point)
                m_currentIndex = pointCount; // Virtual index beyond last point
                m_progress = (m_currentTime - lastPointTime) / m_loopTimeOffset;
                m_progress = std::clamp(m_progress, 0.0f, 1.0f);
                return;
            }
            
            // Find the appropriate point index for current time
            size_t targetIndex = 0;
            for (size_t i = 0; i < pointCount; ++i) {
                const auto& point = GetPoint(i);
                if (m_currentTime <= point.m_transition.m_time) {
                    targetIndex = i;
                    break;
                }
                targetIndex = i + 1;
            }
            
            if (targetIndex >= pointCount) {
                targetIndex = pointCount - 1;
                m_currentIndex = targetIndex;
                m_progress = 1.0f;
                return;
            }
            
            m_currentIndex = targetIndex;
            const auto& currentPoint = GetPoint(targetIndex);
            
            // Calculate progress within current segment
            float prevTime = (targetIndex > 0) ? GetPoint(targetIndex - 1).m_transition.m_time : 0.0f;
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
        float GetCurrentTime() const { return m_currentTime; }
        bool IsPlaying() const { return m_isPlaying; }
        bool IsPaused() const { return m_isPaused; }
        
        // Get timeline duration including loop offset if applicable
        float GetTimelineDuration() const {
            size_t pointCount = GetPointCount();
            if (pointCount == 0) {
                return 0.0f;
            }
            float lastPointTime = GetPoint(pointCount - 1).m_transition.m_time;
            // In loop mode, add offset to create interpolation time from last to first point
            if (m_playbackMode == PlaybackMode::kLoop) {
                return lastPointTime + m_loopTimeOffset;
            }
            return lastPointTime;
        }
        
        void StartPlayback() {
            m_isPlaying = true;
            m_isPaused = false;
        }
        
        void StopPlayback() {
            m_isPlaying = false;
            m_isPaused = false;
        }
        
        void PausePlayback() {
            m_isPaused = true;
        }
        
        void ResumePlayback() {
            m_isPaused = false;
        }
        
        // Set playback mode (from file during import)
        void SetPlaybackMode(PlaybackMode a_mode) {
            m_playbackMode = a_mode;
        }
        
        // Set loop time offset (from file during import)
        void SetLoopTimeOffset(float a_offset) {
            m_loopTimeOffset = a_offset;
        }
        
        // Get playback mode (for export)
        PlaybackMode GetPlaybackMode() const {
            return m_playbackMode;
        }
        
        // Get loop time offset (for export)
        float GetLoopTimeOffset() const {
            return m_loopTimeOffset;
        }
        
        void ResetTimeline() {
            m_currentTime = 0.0f;
            m_currentIndex = 0;
            m_progress = 0.0f;
            m_isPaused = false;
        }
        
    private:
        auto GetPointLinear() const -> decltype(std::declval<PointType>().m_point) {
            const size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return PointType{}.m_point;
            }
            
            size_t currentIdx = m_currentIndex;
            
            // Handle virtual loop segment: treat as interpolating to point 0
            bool isVirtualSegment = (m_playbackMode == PlaybackMode::kLoop && currentIdx == pointCount);
            if (isVirtualSegment) {
                currentIdx = 0;
            }
            
            if (currentIdx >= pointCount) {
                currentIdx = pointCount - 1;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            
            // For virtual segment or normal segments starting after point 0
            if (currentIdx == 0 && !isVirtualSegment) {
                return currentPoint.m_point;
            }
            
            // Get previous point (for virtual segment, it's the last point)
            const auto& prevPoint = isVirtualSegment ? GetPoint(pointCount - 1) : GetPoint(currentIdx - 1);
            
            if (prevPoint.IsNearlyEqual(currentPoint)) {
                return currentPoint.m_point;
            }
            
            float t = _ts_SKSEFunctions::ApplyEasing(m_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
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
            
            // Handle virtual loop segment: treat as interpolating to point 0
            bool isVirtualSegment = (m_playbackMode == PlaybackMode::kLoop && currentIdx == pointCount);
            if (isVirtualSegment) {
                currentIdx = 0;
            }
            
            if (currentIdx >= pointCount) {
                return GetPoint(pointCount - 1).m_point;
            }
            
            // For virtual segment or normal segments starting after point 0
            if (currentIdx == 0 && !isVirtualSegment) {
                return GetPoint(0).m_point;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            // Get previous point (for virtual segment, it's the last point)
            const auto& prevPoint = isVirtualSegment ? GetPoint(pointCount - 1) : GetPoint(currentIdx - 1);
            
            if (prevPoint.IsNearlyEqual(currentPoint)) {
                return prevPoint.m_point;
            }
            
            // Get neighboring points for tangent computation
            PointType pt0, pt1, pt2, pt3;
            
            if (m_playbackMode == PlaybackMode::kLoop) {
                // Loop mode: modulo all indices to wrap around
                pt0 = GetPoint((currentIdx - 2 + pointCount) % pointCount);
                pt1 = GetPoint((currentIdx - 1 + pointCount) % pointCount);
                pt2 = GetPoint(currentIdx % pointCount);
                pt3 = GetPoint((currentIdx + 1) % pointCount);
            } else {
                // End mode: use boundary clamping
                bool hasP0 = currentIdx >= 2;
                bool hasP3 = currentIdx + 1 < pointCount;
                
                pt0 = hasP0 ? GetPoint(currentIdx - 2) : prevPoint;
                pt1 = prevPoint;
                pt2 = currentPoint;
                pt3 = hasP3 ? GetPoint(currentIdx + 1) : currentPoint;
            }
            
            float t = _ts_SKSEFunctions::ApplyEasing(m_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            PointType result = pt1.CubicHermite(pt0, pt1, pt2, pt3, t);
            return result.m_point;
        }
        
        PathType m_path;  //TranslationPath or RotationPath
        
        float m_currentTime = 0.0f;      // Current position in timeline (seconds)
        size_t m_currentIndex = 0;       // Current point index
        float m_progress = 0.0f;         // 0.0 to 1.0 within current segment
        bool m_isPlaying = false;        // Whether timeline is actively playing
        bool m_isPaused = false;         // Whether timeline is paused
        PlaybackMode m_playbackMode = PlaybackMode::kEnd; // Loop or stop at end (set from file)
        float m_loopTimeOffset = 0.0f;   // Extra time after last point for loop interpolation
    };
    
    using TranslationTimeline = Timeline<TranslationPath>;
    using RotationTimeline = Timeline<RotationPath>;
} // namespace FCSE

