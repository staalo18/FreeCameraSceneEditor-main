#pragma once

namespace FCSE {
    class TimelineManager {
        public:
            static TimelineManager& GetSingleton() {
                static TimelineManager instance;
                return instance;
            }
            TimelineManager(const TimelineManager&) = delete;
            TimelineManager& operator=(const TimelineManager&) = delete;

            void Initialize();
            void Update();
            size_t GetTimelineID();
            
            size_t RegisterTimeline();
            bool UnregisterTimeline();
            size_t CycleUp();
            size_t CycleDown();

        private:
            TimelineManager() = default;
            ~TimelineManager() = default;

            void DrawTimeline();

            size_t m_currentTimelineID = 0;
    }; // class TimelineManager
} // namespace FCSE