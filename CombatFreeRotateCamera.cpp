#include "CombatFreeRotateCamera.h"
#include "utils.h"

/// Original Idea From iRetrospect. Thanks.

static CombatFreeRotateCamera combatCamera;

RE::BSEventNotifyControl CombatFreeRotateCamera::ProcessEvent(const SKSE::ActionEvent* a_event, RE::BSTEventSource<SKSE::ActionEvent>* a_eventSource)
{
	if (
		a_event->actor == RE::PlayerCharacter::GetSingleton()
		&& a_event->type == SKSE::ActionEvent::Type::kEndDraw
		)
	{
		auto task = SKSE::GetTaskInterface();
		task->AddTask([]() {
			auto camera = RE::PlayerCamera::GetSingleton();
			if (camera != nullptr)
			{
				auto thirdpersoncam2 = static_cast<RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());

				if (thirdpersoncam2 != nullptr && thirdpersoncam2 == camera->currentState.get())
				{
					if (RE::PlayerCamera::GetSingleton()->cameraTarget.get()->IsWeaponDrawn())
					{
						if (!camera->isWeapSheathed)
						{
							camera->isWeapSheathed = true;
							camera->isProcessed = false;
						}
					}
				}
			}
			});
	}
	return RE::BSEventNotifyControl::kContinue;
}

void CombatFreeRotateCamera::Install()
{
	int enabled = GetPrivateProfileInt("General", "iCombatFreeRotateCamera", 0, INIPATH);
	if (enabled != 0)
	{
		SKSE::GetActionEventSource()->AddEventSink(&combatCamera);
	}
}