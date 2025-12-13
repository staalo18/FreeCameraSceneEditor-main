#pragma once


namespace FCSE {
    constexpr float EPSILON_COMPARISON = 0.0001f;
    
    void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11);

    template<typename PointType>
    PointType CubicHermite(const PointType& p0, const PointType& p1,
                           const PointType& p2, const PointType& p3, float t) {
        // Unwrap relative to p1 (for angular types; no-op for translation)
        PointType p0_unwrapped = p0.UnWrap(p1);
        PointType p2_unwrapped = p2.UnWrap(p1);
        PointType p3_unwrapped = p3.UnWrap(p1);

        // Compute Catmull-Rom tangents in unwrapped space
        PointType m1 = (p2_unwrapped - p0_unwrapped) * 0.5f;
        PointType m2 = (p3_unwrapped - p1) * 0.5f;

        // Get Hermite basis functions
        float h00, h10, h01, h11;
        ComputeHermiteBasis(t, h00, h10, h01, h11);

        // Interpolate in unwrapped space
        PointType result = p1 * h00 + m1 * h10 + p2_unwrapped * h01 + m2 * h11;

        // Wrap result back to valid range
        result.Wrap();
        return result;
    }

    void SetHUDMenuVisible(bool a_visible);

    // Parse timeline file and call callback for each matching section
    bool ParseFCSETimelineFileSections(
        std::ifstream& a_file,
        const std::string& a_sectionName,
        std::function<void(const std::map<std::string, std::string>&)> a_processSection
    );
} // namespace FCSE

