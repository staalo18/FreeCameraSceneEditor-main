#include "Hooks.h"
#include "ControlsManager.h"
#include "APIManager.h"
#include "TimelineManager.h"

namespace FCSE {
    namespace Interface {
        int GetFCSEPluginVersion(RE::StaticFunctionTag*) {
            return 1;
        }
        
        size_t AddTranslationPoint(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut) {
                        
            return FCSE::TimelineManager::GetSingleton().AddTranslationPoint( a_time, a_easeIn, a_easeOut);
log::info("{}: Added translation point with time={}, easeIn={}, easeOut={}", __FUNCTION__, 
                     a_time, a_easeIn, a_easeOut);
        }

        size_t AddRotationPoint(RE::StaticFunctionTag*, float a_time, bool a_easeIn, bool a_easeOut) {
            
            return FCSE::TimelineManager::GetSingleton().AddRotationPoint( a_time, a_easeIn, a_easeOut);
log::info("{}: Added rotation point with time={}, easeIn={}, easeOut={}", __FUNCTION__, 
                     a_time, a_easeIn, a_easeOut);
        }

        bool FCSEFunctions(RE::BSScript::Internal::VirtualMachine * a_vm){
            a_vm->RegisterFunction("GetFCSEPluginVersion", "_ts_FCSE_PapyrusFunctions", GetFCSEPluginVersion);
            a_vm->RegisterFunction("AddTranslationPoint", "_ts_FCSE_PapyrusFunctions", AddTranslationPoint);
            a_vm->RegisterFunction("AddRotationPoint", "_ts_FCSE_PapyrusFunctions", AddRotationPoint);
            return true;
        }
    } // namespace Interface
} // namespace FCSE

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
    log::info("{}: LogLevel: {}, FCSE Plugin version: {}", __FUNCTION__, logLevel, FCSE::Interface::GetFCSEPluginVersion(nullptr));

    if (!SKSE::GetPapyrusInterface()->Register(FCSE::Interface::FCSEFunctions)) {
        log::warn("{}: Failed to register Papyrus functions.", __FUNCTION__);
        return false;
    } else {
        log::info("{}: Registered Papyrus functions", __FUNCTION__);
    }

    SKSE::AllocTrampoline(64);
        
    log::info("{}: Calling Install Hooks", __FUNCTION__);

    Hooks::Install();

    return true;
}
