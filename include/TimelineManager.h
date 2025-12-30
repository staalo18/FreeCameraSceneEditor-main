#pragma once

#include "Timeline.h"

namespace FCSE {
    class TimelineManager {
        public:
            static TimelineManager& GetSingleton() {
                static TimelineManager instance;
                return instance;
            }
            TimelineManager(const TimelineManager&) = delete;
            TimelineManager& operator=(const TimelineManager&) = delete;

            void Update();

            void StartRecording();
            
            void StopRecording();

            size_t AddTranslationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            size_t AddTranslationPoint(float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            size_t AddTranslationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetX, float a_offsetY, float a_offsetZ, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            size_t AddRotationPointAtCamera(float a_time, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            size_t AddRotationPoint(float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            size_t AddRotationPointAtRef(float a_time, RE::TESObjectREFR* a_reference, float a_offsetPitch, float a_offsetYaw, bool a_isOffsetRelative, bool a_easeIn, bool a_easeOut, InterpolationMode a_interpolationMode);
            
            void RemoveTranslationPoint(size_t a_index);
            void RemoveRotationPoint(size_t a_index);

            void ClearTimeline(bool a_notifyUser = true);
            
            size_t GetTranslationPointCount() const { return m_timeline.GetTranslationPointCount(); }
            size_t GetRotationPointCount() const { return m_timeline.GetRotationPointCount(); }
            
            void StartPlayback(float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f);
            void StopPlayback();
            bool IsPlaybackRunning() const { return m_isPlaybackRunning; }
            void PausePlayback();
            void ResumePlayback();
            bool IsPlaybackPaused() const;
            void SetUserTurning(bool a_turning);
            void AllowUserRotation(bool a_allow) { m_allowUserRotation = a_allow; }
            bool IsUserRotationAllowed() const { return m_allowUserRotation; }    
                                    
            bool AddTimelineFromFile(const char* a_filePath, float a_timeOffset = 0.0f);
            bool ExportTimeline(const char* a_filePath) const;

        private:
            TimelineManager() = default;
            ~TimelineManager() = default;

            size_t AddTranslationPoint(const TranslationPoint& a_point);
            size_t AddRotationPoint(const RotationPoint& a_point);
            
            void DrawTimeline();
             
            void RecordTimeline();
            
            void PlayTimeline();

            float GetTimelineDuration() const;

            Timeline m_timeline;
            
            // Recording
            bool m_isRecording = false;
            float m_currentRecordingTime = 0.0f;
            float m_recordingInterval = 1.0f;  // Time between recorded points
            float m_lastRecordedPointTime = 0.0f;

            // Playback
            bool m_isPlaybackRunning = false;
            float m_playbackSpeed = 1.0f;        // Speed multiplier
            bool m_globalEaseIn = false;          // Apply ease-in at start
            bool m_globalEaseOut = false;         // Apply ease-out at end
            float m_playbackDuration = 0.0f;     // Total intended duration
            bool m_isShowingMenus = true;         // Whether menus were showing before playback started
            bool m_showMenusDuringPlayback = false; // Whether to show menus during playback
            bool m_userTurning = false;           // Whether user is manually controlling camera during playback
            bool m_allowUserRotation = false;     // Whether to allow user to turn camera during playback
            RE::BSTPoint2<float> m_rotationOffset; // Offset to apply to rotation to account for user turning
            RE::NiPoint2 m_lastFreeRotation;           // camera free rotation before playback started (third-person only)
    }; // class TimelineManager
} // namespace FCSE