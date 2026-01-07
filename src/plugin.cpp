#include "ControlsManager.h"
#include "APIManager.h"
#include "TimelineManager.h"
#include "Hooks.h"
#include "_ts_SKSEFunctions.h"

/******************************************************************************************/
void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
	// Try requesting APIs at multiple steps to try to work around the SKSE messaging bug
	switch (a_msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPostPostLoad:
		APIs::RequestAPIs();
		break;
	case SKSE::MessagingInterface::kPreLoadGame:
		break;
	case SKSE::MessagingInterface::kPostLoadGame:
	case SKSE::MessagingInterface::kNewGame:
		APIs::RequestAPIs();
        if (auto input = RE::BSInputDeviceManager::GetSingleton()) {
            input->AddEventSink(&FCSE::ControlsManager::GetSingleton());
        } else {
                log::warn("{}: BSInputDeviceManager not available", __FUNCTION__);
        }   
        
        FCSE::TimelineManager::GetSingleton().Initialize();
		break;
	}
}
/******************************************************************************************/
SKSEPluginInfo(
    .Version = Plugin::VERSION,
    .Name = Plugin::NAME,
    .Author = "Staalo",
    .RuntimeCompatibility = SKSE::PluginDeclaration::RuntimeCompatibility(SKSE::VersionIndependence::AddressLibrary),
    .MinimumSKSEVersion = { 2, 2, 3 } // or 0 if you want to support all
)

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* skse) {
    long logLevel = _ts_SKSEFunctions::GetValueFromINI(nullptr, 0, "LogLevel:Log", "SKSE/Plugins/FreeCameraSceneEditor.ini", 3L);
    bool isLogLevelValid = true;
    if (logLevel < 0 || logLevel > 6) {
        logLevel = 2L; // info
        isLogLevelValid = false;
    }

	_ts_SKSEFunctions::InitializeLogging(static_cast<spdlog::level::level_enum>(logLevel));

    Init(skse);
    auto messaging = SKSE::GetMessagingInterface();
	if (!messaging->RegisterListener("SKSE", MessageHandler)) {
		return false;
	}

    if (!isLogLevelValid) {
        log::warn("{}: LogLevel in INI file is invalid. Defaulting to info level.", __FUNCTION__);
    }
//    log::info("{}: LogLevel: {}, FCSE Plugin version: {}", __FUNCTION__, logLevel, FCSE::Interface::GetFCSEPluginVersion(nullptr));


    SKSE::AllocTrampoline(64);
        
    log::info("{}: Calling Install Hooks", __FUNCTION__);

    Hooks::Install();

    return true;
}

