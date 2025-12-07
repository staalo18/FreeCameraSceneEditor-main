#include "Hooks.h"
#include "CameraPathManager.h"

namespace Hooks
{
	void Install()
	{
		log::info("Hooking...");

		MainUpdateHook::Hook();

		log::info("...success");
	}

	void MainUpdateHook::Nullsub()
	{
		_Nullsub();

		FCSE::CameraPathManager::GetSingleton().Update();
	}
} // namespace Hooks
