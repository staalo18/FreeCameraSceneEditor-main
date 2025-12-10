#pragma once

namespace FCSE {
    enum class InterpolationType {
        kOff,      // Instantly set to this value (no interpolation)
        kOn,       // Interpolate to this value over time
        kInvalid   // Skip this component
    };

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
        InterpolationType m_interpolationType;   // How should the transition interpolate between initial and target path points
        float m_time;             // Absolute time at which this point is reached (0 = start of traversal)
        bool m_easeIn;            // Ease in (accelerate from 0 at initial point
        bool m_easeOut;           // Ease out (decelerate to 0 at target point)
        
        Transition() 
            : m_interpolationType(InterpolationType::kInvalid), m_time(0.0f), m_easeIn(false), m_easeOut(false) {}
        
        Transition(InterpolationType a_type, float a_time, bool a_easeIn, bool a_easeOut) 
            : m_interpolationType(a_type), m_time(a_time), m_easeIn(a_easeIn), m_easeOut(a_easeOut) {}
    };
} // namespace FCSE
