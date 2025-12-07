#pragma once
#include <vector>

namespace FCSE {
    
    enum InterpolationType {
        kSet,
        kTranslate,
        kRotate,
        kInvalid
    };

    enum class InterpolationMode {
        kNone,
        kLinear,
        kCatmullRom,
        kBezier
    };
    
    class CameraPathPoint {
    public:
            CameraPathPoint()
            : type(kInvalid), position({0.f, 0.f, 0.f}), rotation({0.f, 0.f}), time(0.f), easeIn(false), easeOut(false) {}

            CameraPathPoint(InterpolationType a_type,
                        const RE::NiPoint3& a_position,
                        const RE::BSTPoint2<float>& a_rotation,
                        float a_time,
                        bool a_easeIn,
                        bool a_easeOut)
            : type(a_type), position(a_position), rotation(a_rotation), time(a_time), easeIn(a_easeIn), easeOut(a_easeOut) {}
       
        // members
        InterpolationType type;  // How the camera should approach this point
        RE::NiPoint3 position; // target position after time seconds
        RE::BSTPoint2<float> rotation; // target rotation after time seconds
        float time; // duration to reach this point from the previous point
        bool easeIn; // whether to ease in (ie accelerate from 0) the transition from the previous point
        bool easeOut; // whether to ease out (ie decelerate to 0) the transition to this point
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

            void SetHUDMenuVisible(bool a_visible);

            void StartCameraMovement();

            // Add a new point to the end of the path
            void AddPoint(InterpolationType a_type);
            
            // Insert point at specific index
            void InsertPoint(size_t index, const CameraPathPoint& point);
            
            // Remove point
            void RemovePoint(size_t index);
            void ClearPath();
            
            // Access points
            const CameraPathPoint* GetPoint(size_t index) const;
            size_t GetPointCount() const { return m_pathPoints.size(); }
            
            // Traversal
            void StartTraversal();
            void StopTraversal();
            bool IsTraversing() const { return m_isTraversing; }
            
            CameraPathPoint GetCurrentPoint() const;

        private:
            CameraPathManager() = default;
            ~CameraPathManager() = default;

            void AddPoint(const CameraPathPoint& point);

            void DrawPath();
             
            void TraverseCamera();

            CameraPathPoint GetPointLinear() const;

            CameraPathPoint GetPointCatmullRom() const;
            
            // Path data
            std::vector<CameraPathPoint> m_pathPoints;
            InterpolationMode m_interpolationMode = InterpolationMode::kCatmullRom;
            
            // Traversal state
            bool m_isTraversing = false;
            size_t m_currentIndex = 0;
            float m_currentSegmentProgress = 0.0f;  // 0.0 to 1.0 within current segment

    }; // class CameraPathManager
} // namespace FCSE