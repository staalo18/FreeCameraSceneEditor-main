#pragma once
#include <vector>

namespace FCSE {
    
    enum InterpolationType {
        kOff,      // Instantly set to this value (no interpolation)
        kOn,       // Interpolate to this value over time
        kInvalid   // Skip this component
    };

    enum class InterpolationMode {
        kNone,
        kLinear,
        kCatmullRom,
        kBezier
    };
    
    enum class TimelineType {
        kTranslation,
        kRotation
    };
    
    struct Transition {
        InterpolationType m_interpolationType;   // How should the transition interpolate between initial and target path points
        float m_time;             // Absolute time at which this point is reached (0 = start of traversal)
        bool m_easeIn;            // Ease in (accelerate from 0 at initial point
        bool m_easeOut;           // Ease out (decelerate to 0 at target point)
        
        Transition() 
            : m_interpolationType(kInvalid), m_time(0.0f), m_easeIn(false), m_easeOut(false) {}
        
        Transition(InterpolationType a_type, float a_time, bool a_easeIn, bool a_easeOut) 
            : m_interpolationType(a_type), m_time(a_time), m_easeIn(a_easeIn), m_easeOut(a_easeOut) {}
    };

    class CameraTranslationPoint {
    public:
        CameraTranslationPoint()
            : m_transitionTranslate(kInvalid, 0.0f, false, false)
            , m_position({0.f, 0.f, 0.f}) {}

        CameraTranslationPoint(const Transition& a_translation,
                        const RE::NiPoint3& a_position)
            : m_transitionTranslate(a_translation)
            , m_position(a_position) {}
       
        // Members
        Transition m_transitionTranslate;
        RE::NiPoint3 m_position;
    };
    
    class CameraRotationPoint {
    public:
        CameraRotationPoint()
            : m_transitionRotate(kInvalid, 0.0f, false, false)
            , m_rotation({0.f, 0.f}) {}

        CameraRotationPoint(const Transition& a_rotation,
                        const RE::BSTPoint2<float>& a_rotationValue)
            : m_transitionRotate(a_rotation)
            , m_rotation(a_rotationValue) {}
       
        // Members
        Transition m_transitionRotate;
        RE::BSTPoint2<float> m_rotation;
    };

    class CameraPathManager {
        public:
            static CameraPathManager& GetSingleton() {
                static CameraPathManager instance;
                return instance;
            }
            CameraPathManager(const CameraPathManager&) = delete;
            CameraPathManager& operator=(const CameraPathManager&) = delete;

            void Update();

            void StartRecording();
            
            void StopRecording();

            // Add a new point to the end of the path
            void AddTranslationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            void AddRotationPoint(float a_time, bool a_easeIn, bool a_easeOut);
            
            // Insert point at specific index
            void InsertTranslationPoint(size_t a_index, const CameraTranslationPoint& a_point);
            void InsertRotationPoint(size_t a_index, const CameraRotationPoint& a_point);
            
            // Remove point
            void RemoveTranslationPoint(size_t a_index);
            void RemoveRotationPoint(size_t a_index);

            void ClearPath(bool a_notify = true);
            
            // Access points
            const CameraTranslationPoint* GetTranslationPoint(size_t a_index) const;
            const CameraRotationPoint* GetRotationPoint(size_t a_index) const;
            size_t GetTranslationPointCount() const { return m_translationPoints.size(); }
            size_t GetRotationPointCount() const { return m_rotationPoints.size(); }
            
            // Traversal
            void StartTraversal();
            void StopTraversal();
            bool IsTraversing() const { return m_isTraversing; }
            
            // Import/Export camera path from/to file
            bool ImportCameraPath(const char* a_filePath);
            bool ExportCameraPath(const char* a_filePath) const;

        private:
            CameraPathManager() = default;
            ~CameraPathManager() = default;

            void AddTranslationPoint(const CameraTranslationPoint& a_point);
            void AddRotationPoint(const CameraRotationPoint& a_point);
            
            // Template helper for validating and inserting points in sorted order
            template<typename PointType>
            void InsertPointSorted(
                std::vector<PointType>& a_points,
                Transition PointType::* a_transitionMember,
                const PointType& a_point);
            
            // Template helper for adding points with absolute time calculation
            template<typename PointType, typename ValueType>
            void AddPoint(
                void (CameraPathManager::*a_addFunction)(const PointType&),
                const ValueType& a_value,
                float a_time,
                bool a_easeIn,
                bool a_easeOut);

            void SetHUDMenuVisible(bool a_visible);

            void DrawPath();
             
            void RecordPath();
            
            void TraverseCamera();
            
            void ProcessTimeline(TimelineType a_timelineType);
            
            template<typename PointType>
            void ProcessTimeline(
                const std::vector<PointType>& a_points,
                size_t& a_currentIndex,
                float& a_progress,
                Transition PointType::* a_transitionMember);

            // Get current interpolated position
            RE::NiPoint3 GetCurrentPosition() const;
            
            // Get current interpolated rotation
            RE::BSTPoint2<float> GetCurrentRotation() const;
            
            // Linear interpolation helpers
            RE::NiPoint3 GetTranslationLinear() const;
            RE::BSTPoint2<float> GetRotationLinear() const;
            
            // Cubic Hermite interpolation helpers
            RE::NiPoint3 GetTranslationCubicHermite() const;
            RE::BSTPoint2<float> GetRotationCubicHermite() const;
            
            // Path data
//            std::vector<CameraPathPoint> m_pathPoints;
            std::vector<CameraTranslationPoint> m_translationPoints;
            std::vector<CameraRotationPoint> m_rotationPoints;

            InterpolationMode m_interpolationMode = InterpolationMode::kCatmullRom;
            
            // Recording
            bool m_isRecording = false;
            float m_currentRecordingTime = 0.0f;
            float m_recordingInterval = 1.0f;  // Time between recorded points
            float m_lastRecordedPointTime = 0.0f;

            // Traversal
            bool m_isTraversing = false;            
            float m_currentTraversalTime = 0.0f;  // Absolute time since start of traversal
            
            size_t m_currentTranslationIndex = 0;
            float m_transitionTranslateProgress = 0.0f;  // 0.0 to 1.0 within current segment
            
            size_t m_currentRotationIndex = 0;
            float m_transitionRotateProgress = 0.0f;     // 0.0 to 1.0 within current segment

    }; // class CameraPathManager
} // namespace FCSE