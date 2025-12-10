#include "Hooks.h"
#include "TimelineManager.h"

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

		FCSE::TimelineManager::GetSingleton().Update();
	}
} // namespace Hooks
