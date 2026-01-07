#include "APIManager.h"

void APIs::RequestAPIs()
{
    if (!TrueHUD) {
		TrueHUD = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
		if (TrueHUD) {
			log::info("{}: Obtained TrueHUD API - {:x}", __FUNCTION__, reinterpret_cast<uintptr_t>(TrueHUD));
		} else {
			log::info("{}: Failed to obtain TrueHUD API", __FUNCTION__);
		}
	}	
	
	if (!FCFW) {
		FCFW = reinterpret_cast<FCFW_API::IVFCFW1*>(FCFW_API::RequestPluginAPI(FCFW_API::InterfaceVersion::V1));
		if (FCFW) {
			log::info("Obtained FCFW API - {0:x}", reinterpret_cast<uintptr_t>(FCFW));
		} else {
			log::info("Failed to obtain FCFW API");
		}
	}
}

