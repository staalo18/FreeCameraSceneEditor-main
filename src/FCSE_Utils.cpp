#include "FCSE_Utils.h"
#include "APIManager.h"

namespace FCSE {
    void ComputeHermiteBasis(float t, float& h00, float& h10, float& h01, float& h11) {
        float t2 = t * t;
        float t3 = t2 * t;
        
        h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;   // basis for p1
        h10 = t3 - 2.0f * t2 + t;              // basis for m1
        h01 = -2.0f * t3 + 3.0f * t2;          // basis for p2
        h11 = t3 - t2;                          // basis for m2
    }
    
    void SetHUDMenuVisible(bool a_visible) {
        if (!APIs::TrueHUD) {
            return;
        }

        auto* ui = RE::UI::GetSingleton();
        if (ui) {
            auto trueHUDMenu = ui->GetMenu<RE::IMenu>("TrueHUD");
            if (trueHUDMenu && trueHUDMenu->uiMovie) {
                trueHUDMenu->uiMovie->SetVisible(a_visible);
            }
        }        
    }

    bool ParseFCSETimelineFileSections(
        std::ifstream& a_file,
        const std::string& a_sectionName,
        std::function<void(const std::map<std::string, std::string>&)> a_processSection
    ) {
        if (!a_file.is_open()) {
            return false;
        }

        std::string line;
        std::string currentSection;
        std::map<std::string, std::string> currentData;

        while (std::getline(a_file, line)) {
            // Trim whitespace
            line.erase(0, line.find_first_not_of(" \t\r\n"));
            line.erase(line.find_last_not_of(" \t\r\n") + 1);
            
            // Skip empty lines and comments
            if (line.empty() || line[0] == ';' || line[0] == '#') {
                continue;
            }
            
            // Check for section header
            if (line[0] == '[' && line.back() == ']') {
                // Process previous section if it matches
                if (currentSection == a_sectionName) {
                    a_processSection(currentData);
                }

                // Start new section
                currentSection = line.substr(1, line.length() - 2);
                currentData.clear();
                continue;
            }
            
            // Parse key=value pairs
            size_t eqPos = line.find('=');
            if (eqPos != std::string::npos) {
                std::string key = line.substr(0, eqPos);
                std::string value = line.substr(eqPos + 1);
                
                // Trim key and value
                key.erase(0, key.find_first_not_of(" \t"));
                key.erase(key.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // Remove comments from value
                size_t commentPos = value.find(';');
                if (commentPos != std::string::npos) {
                    value = value.substr(0, commentPos);
                    value.erase(value.find_last_not_of(" \t") + 1);
                }
                
                currentData[key] = value;
            }
        }
        
        // Process last section if it matches
        if (currentSection == a_sectionName) {
            a_processSection(currentData);
        }

        return true;
    }
} // namespace FCSE
