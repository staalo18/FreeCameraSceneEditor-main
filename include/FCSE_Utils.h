#pragma once


namespace FCSE {

    void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11);

    float ComputeTangent(float v0, float v1, float v2, bool hasV0, bool hasV2);

    void SetHUDMenuVisible(bool a_visible);

    // Parse timeline file and call callback for each matching section
    bool ParseFCSETimelineFileSections(
        std::ifstream& a_file,
        const std::string& a_sectionName,
        std::function<void(const std::map<std::string, std::string>&)> a_processSection
    );
} // namespace FCSE

