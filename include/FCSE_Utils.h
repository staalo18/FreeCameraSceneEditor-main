#pragma once


namespace FCSE {
    constexpr float EPSILON_COMPARISON = 0.0001f;
    
    void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11);

    float CubicHermiteInterpolate(float a0, float a1, float a2, float a3, float t);

    float CubicHermiteInterpolateAngular(float a0, float a1, float a2, float a3, float t);

    void SetHUDMenuVisible(bool a_visible);

    // Parse timeline file and call callback for each matching section
    bool ParseFCSETimelineFileSections(
        std::ifstream& a_file,
        const std::string& a_sectionName,
        std::function<void(const std::map<std::string, std::string>&)> a_processSection
    );
} // namespace FCSE

