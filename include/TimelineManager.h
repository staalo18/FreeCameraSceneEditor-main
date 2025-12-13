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

            // Add a new point at time a_time
            size_t AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            size_t AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            
            // Update existing point (returns new index after potential re-sorting)
            size_t EditTranslationPoint(size_t a_index, float a_time, float a_posX, float a_posY, float a_posZ, bool a_easeIn, bool a_easeOut);
            size_t EditRotationPoint(size_t a_index, float a_time, float a_pitch, float a_yaw, bool a_easeIn, bool a_easeOut);
            
            // Remove point
            void RemoveTranslationPoint(size_t a_index);
            void RemoveRotationPoint(size_t a_index);

            void ClearTimeline(bool a_notifyUser = true);
            
            // Access points
            const TranslationPoint& GetTranslationPoint(size_t a_index) const;
            const RotationPoint& GetRotationPoint(size_t a_index) const;
            size_t GetTranslationPointCount() const { return m_translationTimeline.GetPointCount(); }
            size_t GetRotationPointCount() const { return m_rotationTimeline.GetPointCount(); }
            
            // Traversal
            void StartTraversal(float a_speed = 1.0f, bool a_globalEaseIn = false, bool a_globalEaseOut = false, bool a_useDuration = false, float a_duration = 0.0f);
            void StopTraversal();
            bool IsTraversing() const { return m_isTraversing; }
                        
            // Import/Export camera timeline from/to file
            bool ImportTimeline(const char* a_filePath);
            bool ExportTimeline(const char* a_filePath) const;

        private:
            TimelineManager() = default;
            ~TimelineManager() = default;

            size_t AddTranslationPoint(const TranslationPoint& a_point);
            size_t AddRotationPoint(const RotationPoint& a_point);
            
            size_t EditTranslationPoint(size_t a_index, const TranslationPoint& a_point);
            size_t EditRotationPoint(size_t a_index, const RotationPoint& a_point);
            
            void DrawTimeline();
             
            void RecordTimeline();
            
            void TraverseCamera();

            float GetTimelineDuration() const;

            TranslationTimeline m_translationTimeline;
            RotationTimeline m_rotationTimeline;
            
            // Recording
            bool m_isRecording = false;
            float m_currentRecordingTime = 0.0f;
            float m_recordingInterval = 1.0f;  // Time between recorded points
            float m_lastRecordedPointTime = 0.0f;

            // Traversal
            bool m_isTraversing = false;            
            float m_currentTraversalTime = 0.0f;  // Absolute time since start of traversal
            float m_traversalDuration = 0.0f;     // Total duration of traversal
            float m_traversalSpeed = 1.0f;        // Speed multiplier (for kTimeline mode)
            bool m_globalEaseIn = false;        // Apply ease-in at start (both modes)
            bool m_globalEaseOut = false;       // Apply ease-out at end (both modes)
       
    }; // class TimelineManager
} // namespace FCSE