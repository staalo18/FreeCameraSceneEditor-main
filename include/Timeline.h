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
        using TransitionPoint = typename PathType::TransitionPoint;
        
        Timeline() = default;
        ~Timeline() = default;
        
        size_t AddPoint(const TransitionPoint& a_point) { 
            size_t result = m_path.AddPoint(a_point);
            ResetTimeline();
            return result;
        }
        const TransitionPoint& GetPoint(size_t a_index) const { return m_path.GetPoint(a_index); }
        void RemovePoint(size_t a_index) { 
            m_path.RemovePoint(a_index);
            ResetTimeline();
        }
        void ClearTimeline() { 
            m_path.ClearPath();
            ResetTimeline();
            m_playbackMode = PlaybackMode::kEnd;
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
        TransitionPoint GetPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut) {
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
            
            m_playbackTime += a_deltaTime;
            
            // Get timeline duration (includes loop offset if in loop mode)
            float timelineDuration = GetTimelineDuration();
            
            // Check for completion and handle loop
            if (m_playbackTime >= timelineDuration) {
                if (m_playbackMode == PlaybackMode::kLoop) {
                    // Loop: use modulo to wrap time seamlessly
                    m_playbackTime = std::fmod(m_playbackTime, timelineDuration);
                } else {
                    // End: clamp to final position and stop
                    m_playbackTime = timelineDuration;
                    m_isPlaying = false;
                }
            }
            
        }
        
        float GetPlaybackTime() const { return m_playbackTime; }
        bool IsPlaying() const { return m_isPlaying; }
        bool IsPaused() const { return m_isPaused; }
        
        // Get interpolated point at a specific time (for global easing)
        auto GetPointAtTime(float a_time) const -> decltype(std::declval<TransitionPoint>().m_point) {
            auto pointCount = GetPointCount();
            if (pointCount == 0) {
                return TransitionPoint{}.m_point;
            }
            
            // Calculate state for requested time
            size_t index = 0;
            float progress = 0.0f;

            //  Calculate segment state (index and progress)                
            float lastPointTime = GetPoint(pointCount - 1).m_transition.m_time;
            
            // Check if we're in the virtual loop segment (after last point)
            if (m_playbackMode == PlaybackMode::kLoop && m_loopTimeOffset > 0.0f && a_time > lastPointTime) {
                index = pointCount;  // Virtual index beyond last point
                progress = (a_time - lastPointTime) / m_loopTimeOffset;
                progress = std::clamp(progress, 0.0f, 1.0f);
            } else { // Find the segment containing this time
                size_t targetIndex = 0;
                for (size_t i = 0; i < pointCount; ++i) {
                    const auto& point = GetPoint(i);
                    if (a_time <= point.m_transition.m_time) {
                        targetIndex = i;
                        break;
                    }
                    targetIndex = i + 1;
                }
                
                if (targetIndex >= pointCount) {
                    targetIndex = pointCount - 1;
                    index = targetIndex;
                    progress = 1.0f;
                } else {   
                    // Calculate progress within this segment
                    index = targetIndex;
                    
                    if (targetIndex > 0) {
                        const auto& prevPoint = GetPoint(targetIndex - 1);
                        const auto& currentPoint = GetPoint(targetIndex);
                        float segmentDuration = currentPoint.m_transition.m_time - prevPoint.m_transition.m_time;
                        if (segmentDuration > 0.0f) {
                            progress = (a_time - prevPoint.m_transition.m_time) / segmentDuration;
                            progress = std::clamp(progress, 0.0f, 1.0f);
                        } else {
                            progress = 1.0f;
                        }
                    } else {
                        progress = 0.0f;
                    }
                }
            }
            
            // Get interpolated point for requested time
            return GetInterpolatedPoint(index, progress);
        }
        
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
            m_path.UpdateCameraPoints(); // Store current camera values for kCamera points
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
            m_playbackTime = 0.0f;
            m_isPlaying = false;
            m_isPaused = false;
        }
        
    private:
        // Get interpolated point for given index and progress
        auto GetInterpolatedPoint(size_t a_index, float a_progress) const -> decltype(std::declval<TransitionPoint>().m_point) {
            if (GetPointCount() == 0) {
                return TransitionPoint{}.m_point;
            }
            
            size_t currentIdx = a_index;
            if (currentIdx >= GetPointCount()) {
                currentIdx = GetPointCount() - 1;
            }
            
            const auto& currentPoint = GetPoint(currentIdx);
            
            switch (currentPoint.m_transition.m_mode) {
                case InterpolationMode::kNone:
                    return currentPoint.m_point;
                case InterpolationMode::kLinear:
                    return GetPointLinear(a_index, a_progress);
                case InterpolationMode::kCubicHermite:
                    return GetPointCubicHermite(a_index, a_progress);
                default:
                    return GetPointLinear(a_index, a_progress);
            }
        }
        
        auto GetPointLinear(size_t a_index, float a_progress) const -> decltype(std::declval<TransitionPoint>().m_point) {
            const size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return TransitionPoint{}.m_point;
            }
            
            size_t currentIdx = a_index;
            
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
            
            float t = _ts_SKSEFunctions::ApplyEasing(a_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            TransitionPoint result = prevPoint + (currentPoint - prevPoint) * t;
            return result.m_point;
        }
        
        auto GetPointCubicHermite(size_t a_index, float a_progress) const -> decltype(std::declval<TransitionPoint>().m_point) {
            const size_t pointCount = GetPointCount();
            
            if (pointCount == 0) {
                return TransitionPoint{}.m_point;
            }
            
            if (pointCount == 1) {
                return GetPoint(0).m_point;
            }
            
            size_t currentIdx = a_index;
            
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
            TransitionPoint pt0, pt1, pt2, pt3;
            
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
            
            float t = _ts_SKSEFunctions::ApplyEasing(a_progress,
                                 currentPoint.m_transition.m_easeIn,
                                 currentPoint.m_transition.m_easeOut);
            
            TransitionPoint result = pt1.CubicHermite(pt0, pt1, pt2, pt3, t);
            return result.m_point;
        }
        
        PathType m_path;  //TranslationPath or RotationPath
        
        float m_playbackTime = 0.0f;              // Current position in timeline (seconds)
        bool m_isPlaying = false;                // Whether timeline is actively playing
        bool m_isPaused = false;                 // Whether timeline is paused
        PlaybackMode m_playbackMode = PlaybackMode::kEnd; // Loop or stop at end (set from file)
        float m_loopTimeOffset = 0.0f;           // Extra time after last point for loop interpolation
    };
    
    using TranslationTimeline = Timeline<TranslationPath>;
    using RotationTimeline = Timeline<RotationPath>;
} // namespace FCSE

