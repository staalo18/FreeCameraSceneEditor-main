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
            void AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            void AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            
            // Remove point
            void RemoveTranslationPoint(size_t a_index);
            void RemoveRotationPoint(size_t a_index);

            void ClearTimeline(bool a_notify = true);
            
            // Access points
            const TranslationPoint* GetTranslationPoint(size_t a_index) const;
            const RotationPoint* GetRotationPoint(size_t a_index) const;
            size_t GetTranslationPointCount() const { return m_translationTimeline.GetPointCount(); }
            size_t GetRotationPointCount() const { return m_rotationTimeline.GetPointCount(); }
            
            // Traversal
            void StartTraversal();
            void StopTraversal();
            bool IsTraversing() const { return m_isTraversing; }
            
            // Import/Export camera timeline from/to file
            bool ImportTimeline(const char* a_filePath);
            bool ExportTimeline(const char* a_filePath) const;

        private:
            TimelineManager() = default;
            ~TimelineManager() = default;

            void AddTranslationPoint(const TranslationPoint& a_point);
            void AddRotationPoint(const RotationPoint& a_point);
            
            void DrawTimeline();
             
            void RecordTimeline();
            
            void TraverseCamera();

            TranslationTimeline m_translationTimeline;
            RotationTimeline m_rotationTimeline;

            InterpolationMode m_interpolationMode = InterpolationMode::kCubicHermite;
            
            // Recording
            bool m_isRecording = false;
            float m_currentRecordingTime = 0.0f;
            float m_recordingInterval = 1.0f;  // Time between recorded points
            float m_lastRecordedPointTime = 0.0f;

            // Traversal
            bool m_isTraversing = false;            
            float m_currentTraversalTime = 0.0f;  // Absolute time since start of traversal
        
    }; // class TimelineManager
} // namespace FCSE