#pragma once


namespace FCSE {
    constexpr float EPSILON_COMPARISON = 0.0001f;
    
    void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11);

    void SetHUDMenuVisible(bool a_visible);

    // Parse timeline file and call callback for each matching section
    bool ParseFCSETimelineFileSections(
        std::ifstream& a_file,
        const std::string& a_sectionName,
        std::function<void(const std::map<std::string, std::string>&)> a_processSection
    );
} // namespace FCSE

