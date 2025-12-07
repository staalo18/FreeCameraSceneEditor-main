#pragma once
#include <vector>

namespace FCSE {
    
    struct CameraPathPoint {
        RE::NiPoint3 position;
        RE::BSTPoint2<float> rotation;
        float time;  // Duration to reach this point from previous, or timestamp?
    };

    enum class InterpolationMode {
        Linear,
        CatmullRom,
        Bezier
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
            void AddPoint();
            void AddPoint(const CameraPathPoint& point);
            void AddPoint(const RE::NiPoint3& pos, const RE::BSTPoint2<float>& rot, float time);
            
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

            void DrawPath();
             
            void TraverseCamera();

            CameraPathPoint GetPointLinear() const;

            CameraPathPoint GetPointCatmullRom() const;
            
            // Path data
            std::vector<CameraPathPoint> m_pathPoints;
            InterpolationMode m_interpolationMode = InterpolationMode::CatmullRom;
            
            // Traversal state
            bool m_isTraversing = false;
            size_t m_currentIndex = 0;
            float m_currentSegmentProgress = 0.0f;  // 0.0 to 1.0 within current segment

    }; // class CameraPathManager
} // namespace FCSE