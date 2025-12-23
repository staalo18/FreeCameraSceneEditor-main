#pragma once

namespace FCSE {
    enum class InterpolationMode {
        kNone,
        kLinear,
        kCubicHermite,
        kBezier
    };
    
    enum class TimelineType {
        kTranslation,
        kRotation
    };
    
    struct Transition {
        float m_time;                           // Absolute time at which this point is reached (0 = start of traversal)
        InterpolationMode m_mode;               // Interpolation mode for this segment
        bool m_easeIn;                          // Ease in (accelerate from 0 at initial point)
        bool m_easeOut;                         // Ease out (decelerate to 0 at target point)
        
        Transition() 
            : m_time(0.0f), m_mode(InterpolationMode::kCubicHermite), m_easeIn(false), m_easeOut(false) {}
        
        Transition(float a_time, InterpolationMode a_mode, bool a_easeIn, bool a_easeOut) 
            : m_time(a_time), m_mode(a_mode), m_easeIn(a_easeIn), m_easeOut(a_easeOut) {}
    };

    inline InterpolationMode ToInterpolationMode(int a_mode) {
        if (a_mode < 0 || a_mode > static_cast<int>(InterpolationMode::kBezier)) {
            log::warn("Invalid interpolation mode {} passed, defaulting to kNone", a_mode);
            return InterpolationMode::kNone;
        }
        return static_cast<InterpolationMode>(a_mode);
    }
} // namespace FCSE
