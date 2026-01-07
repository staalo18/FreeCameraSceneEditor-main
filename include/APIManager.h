#pragma once

#include "API/TrueHUDAPI.h"
#include "API/FCFW_API.h"

struct APIs
{
    static inline TRUEHUD_API::IVTrueHUD3* TrueHUD = nullptr;
    static inline FCFW_API::IVFCFW1* FCFW = nullptr;

	static void RequestAPIs();
};
