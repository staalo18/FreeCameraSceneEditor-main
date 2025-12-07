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
        Translation,
        Rotation
    };
    
    struct Transition {
        InterpolationType m_interpolationType;   // How should the transition interpolate between initial and target path points
        float m_time;             // Duration for this transition, ie until next point is reached
        bool m_easeIn;            // Ease in (accelerate from 0 at initial point
        bool m_easeOut;           // Ease out (decelerate to 0 at target point)
        
        Transition() 
            : m_interpolationType(kInvalid), m_time(0.0f), m_easeIn(false), m_easeOut(false) {}
        
        Transition(InterpolationType a_type, float a_time, bool a_easeIn, bool a_easeOut) 
            : m_interpolationType(a_type), m_time(a_time), m_easeIn(a_easeIn), m_easeOut(a_easeOut) {}
    };
    
    class CameraPathPoint {
    public:
        CameraPathPoint()
            : m_transitionTranslate(kInvalid, 0.0f, false, false)
            , m_transitionRotate(kInvalid, 0.0f, false, false)
            , m_position({0.f, 0.f, 0.f})
            , m_rotation({0.f, 0.f}) {}

        CameraPathPoint(const Transition& a_translation,
                        const Transition& a_rotation,
                        const RE::NiPoint3& a_position,
                        const RE::BSTPoint2<float>& a_rotationValue)
            : m_transitionTranslate(a_translation)
            , m_transitionRotate(a_rotation)
            , m_position(a_position)
            , m_rotation(a_rotationValue) {}
       
        // Members
        Transition m_transitionTranslate;
        Transition m_transitionRotate;
        RE::NiPoint3 m_position;
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

            // Add a new point to the end of the path
            void AddPathPoint(TimelineType a_type);

            void AddPathPoint(TimelineType a_type, float a_time, bool a_easeIn, bool a_easeOut);
            
            // Insert point at specific index
            void InsertPoint(size_t a_index, const CameraPathPoint& a_point);
            
            // Remove point
            void RemovePoint(size_t a_index);
            void ClearPath();
            
            // Access points
            const CameraPathPoint* GetPoint(size_t a_index) const;
            size_t GetPointCount() const { return m_pathPoints.size(); }
            
            // Traversal
            void StartTraversal();
            void StopTraversal();
            bool IsTraversing() const { return m_isTraversing; }

        private:
            CameraPathManager() = default;
            ~CameraPathManager() = default;

            void AddPoint(const CameraPathPoint& a_point);

            void SetHUDMenuVisible(bool a_visible);

            void DrawPath();
             
            void TraverseCamera();
            
            void ProcessTimeline(TimelineType a_timelineType, float a_deltaTime);

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
            std::vector<CameraPathPoint> m_pathPoints;
            InterpolationMode m_interpolationMode = InterpolationMode::kCatmullRom;
            
            // Traversal state - separate timelines for translation and rotation
            bool m_isTraversing = false;
            
            size_t m_currentTranslationIndex = 0;
            float m_transitionTranslateProgress = 0.0f;  // 0.0 to 1.0 within current segment
            
            size_t m_currentRotationIndex = 0;
            float m_transitionRotateProgress = 0.0f;     // 0.0 to 1.0 within current segment

    }; // class CameraPathManager
} // namespace FCSE