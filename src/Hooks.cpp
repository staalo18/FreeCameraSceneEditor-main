#include "Hooks.h"
#include "TimelineManager.h"

namespace Hooks
{
	void Install()
	{
		log::info("Hooking...");

		MainUpdateHook::Hook();
		LookHook::Hook();
		MovementHook::Hook();

		log::info("...success");
	}

	void MainUpdateHook::Nullsub()
	{
		_Nullsub();

		FCSE::TimelineManager::GetSingleton().Update();
	}

	void LookHook::ProcessThumbstick(RE::LookHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data)
	{
		auto& timelineManager = FCSE::TimelineManager::GetSingleton();

		if (!RE::UI::GetSingleton()->GameIsPaused()) {
			timelineManager.SetUserTurning(true);
			if (timelineManager.IsPlaybackRunning() && !timelineManager.IsUserRotationAllowed()) {
				return; // ignore look input during playback
			}
		}

		_ProcessThumbstick(a_this, a_event, a_data);
	}

	void LookHook::ProcessMouseMove(RE::LookHandler* a_this, RE::MouseMoveEvent* a_event, RE::PlayerControlsData* a_data)
	{
		auto& timelineManager = FCSE::TimelineManager::GetSingleton();

		if (!RE::UI::GetSingleton()->GameIsPaused()) {
			timelineManager.SetUserTurning(true);
			if (timelineManager.IsPlaybackRunning() && !timelineManager.IsUserRotationAllowed()) {
				return; // ignore look input during playback
			}
		}

		_ProcessMouseMove(a_this, a_event, a_data);
	}

	void MovementHook::ProcessThumbstick(RE::MovementHandler* a_this, RE::ThumbstickEvent* a_event, RE::PlayerControlsData* a_data)
	{
		auto& timelineManager = FCSE::TimelineManager::GetSingleton();
		if (a_event && a_event->IsLeft() && timelineManager.IsPlaybackRunning() && !RE::UI::GetSingleton()->GameIsPaused()) {
			return; // ignore movement input during playback
		}

		_ProcessThumbstick(a_this, a_event, a_data);
	}

	void MovementHook::ProcessButton(RE::MovementHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data)
	{
		bool bRelevant = false;
		if (a_event)
		{
			auto& userEvent = a_event->QUserEvent();
			auto userEvents = RE::UserEvents::GetSingleton();

			if (userEvent == userEvents->forward || 
				userEvent == userEvents->back ||
				userEvent == userEvents->strafeLeft ||
				userEvent == userEvents->strafeRight) {
				bRelevant = a_event->IsPressed();
			}
		}
		auto& timelineManager = FCSE::TimelineManager::GetSingleton();
		if (bRelevant && timelineManager.IsPlaybackRunning() && !RE::UI::GetSingleton()->GameIsPaused()) {
			return; // ignore movement input during playback
		}

		_ProcessButton(a_this, a_event, a_data);
	}
} // namespace Hooks
