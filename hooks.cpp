#include "hooks.h"
#include "utils.h"

void ProcessThumbstickEx(RE::MovementHandler* _this,
	RE::ThumbstickEvent* a_event,
	RE::PlayerControlsData* a_data);

void ProcessButtonEx(RE::MovementHandler* _this,
	RE::ButtonEvent* a_event,
	RE::PlayerControlsData* a_data);

void UpdateEx(RE::ThirdPersonState* tps, RE::BSTSmartPointer<RE::TESCameraState>& a_nextState);

static REL::Offset<decltype(ProcessThumbstickEx)> originProcessThumbstick;
static REL::Offset<decltype(ProcessButtonEx)> originProcessButton;
static REL::Offset<decltype(UpdateEx)> originUpdate;

static float SideMoveRotationInputXMin = 0.7f;
static float SideMoveRotationScale = 0.013f;
static float MovingTurnMax = 0.3f;
static float CommitmentTurnMax = 0.01f;
static int PitchLookControl = 0;
static int ForceFix = 0;

bool Process(RE::NiPoint2* value, RE::PlayerControlsData* a_data, bool forceFix = false)
{
	if (
		!RE::PlayerControls::GetSingleton()->movementHandler->inputEventHandlingEnabled
		|| !RE::PlayerControls::GetSingleton()->lookHandler->inputEventHandlingEnabled
		)
	{
		return false;
	}

	auto camera = RE::PlayerCamera::GetSingleton();
	if (camera != nullptr)
	{
		auto player = RE::PlayerCharacter::GetSingleton();
		auto tps = static_cast<RE::ThirdPersonState*>(camera->cameraStates[RE::CameraState::kThirdPerson].get());
		if (tps != nullptr
			&& player != nullptr
			&& camera->cameraStates[RE::CameraState::kThirdPerson].get() == camera->currentState.get()
			&& tps->toggleAnimCam
			)
		{
			float rotX = value->x;
			if (abs(rotX) < SideMoveRotationInputXMin)
			{
				rotX = 0;
			}

			if (a_data->fovSlideMode)
			{
				a_data->lookInputVec.x += value->x;
				a_data->lookInputVec.y += value->y;
				return true;
			}

			RE::NiPoint2 l; l.x = value->x; l.y = value->y;
			a_data->moveInputVec.x = 0;
			a_data->moveInputVec.y = VectorMagnitude(l);
			l = VectorNormalize(l);

			RE::NiPoint2 r; r.x = 0; r.y = 1;

			float cossita = DotProduct(l, r) / (VectorMagnitude(l) * VectorMagnitude(r));
			if (!isnan(cossita))
			{
				bool bAllowRotation = false;
				player->GetGraphVariableBool("bAllowRotation", bAllowRotation);

				//dmc direction lock.
				if ( player->IsGrabbing() || ForceFix )
				{
					RE::NiPoint2 inputDir; inputDir.x = 0.0f; inputDir.y = 1.0f;
					inputDir = Rotate(
						inputDir,
						NormalAbsoluteAngle(acos(cossita)
							* (CrossProduct(l, r) < 0 ? 1 : -1) - tps->freeRotation.x));
					a_data->moveInputVec.x = inputDir.x;
					a_data->moveInputVec.y = inputDir.y;
				}
				//360move
				else
				{
					const float turnMax = bAllowRotation ? CommitmentTurnMax : MovingTurnMax;

					float rad = NormalAbsoluteAngle(acos(cossita) * (CrossProduct(l, r) < 0 ? -1 : 1));

					float freerot = NormalAbsoluteAngle(-tps->freeRotation.x);
					float diffrad = rad - freerot;
					float absdiffrad = abs(diffrad);
					
					if (absdiffrad > turnMax)
					{
						float turnValue = turnMax;
						if (absdiffrad > M_PI)
						{
							diffrad = -diffrad;
							float diff2 = (M_PI * 2.0f) - absdiffrad;
							turnValue = diff2 < turnMax ? diff2 : turnMax;
						}

						rad = NormalAbsoluteAngle((turnValue * (diffrad < 0.0f ? -1.0f : 1.0f)) + freerot);
					}

					float moveAngle = rad;
					float angleZ = NormalAbsoluteAngle(player->data.angle.z + tps->freeRotation.x + moveAngle);
					float diffAngleZ = -moveAngle;

					player->data.angle.z = angleZ;
					tps->freeRotation.x = diffAngleZ;

					player->data.angle.z += (rotX * SideMoveRotationScale);
				}
			}
			else
			{
				a_data->moveInputVec.y = 0.0f;
			}

			return true;
		}
	}
	return false;
}


// Gamepad Moving Update
void ProcessThumbstickEx(RE::MovementHandler* _this,
	RE::ThumbstickEvent* a_event,
	RE::PlayerControlsData* a_data)
{
	RE::NiPoint2 a_thumbstick_event;
	a_thumbstick_event.x = a_event->xValue;
	a_thumbstick_event.y = a_event->yValue;

	if (!Process(&a_thumbstick_event, a_data))
	{
		originProcessThumbstick(_this, a_event, a_data);
	}
}


// Keyboard Moving Update
static RE::NiPoint2 tempbuttonvec;

void ProcessButtonEx(RE::MovementHandler* _this,
	RE::ButtonEvent* a_event,
	RE::PlayerControlsData* a_data)
{
	RE::NiPoint2 a_thumbstick_event;
	a_thumbstick_event.x = a_thumbstick_event.y = 0;

	if (a_data->moveInputVec.y != 0.0f)
	{
		a_thumbstick_event.x = tempbuttonvec.x;
		a_thumbstick_event.y = tempbuttonvec.y;
		tempbuttonvec.x = 0.0f;
		tempbuttonvec.y = 0.0f;
	}
	if (RE::UserEvents::GetSingleton()->forward == a_event->userEvent)
	{
		a_thumbstick_event.y = a_event->value;
	}
	else if (RE::UserEvents::GetSingleton()->back == a_event->userEvent)
	{
		a_thumbstick_event.y = -a_event->value;
	}
	else if (RE::UserEvents::GetSingleton()->strafeLeft == a_event->userEvent)
	{
		a_thumbstick_event.x = -a_event->value;
	}
	else if (RE::UserEvents::GetSingleton()->strafeRight == a_event->userEvent)
	{
		a_thumbstick_event.x = a_event->value;
	}

	if (a_data->moveInputVec.y == 0.0f)
	{
		tempbuttonvec.x = a_thumbstick_event.x;
		tempbuttonvec.y = a_thumbstick_event.y;
	}

	if (!Process(&a_thumbstick_event, a_data))
	{
		originProcessButton(_this, a_event, a_data);
	}
}

//Rotation Update
void UpdateEx(RE::ThirdPersonState* tps, RE::BSTSmartPointer<RE::TESCameraState>& a_nextState)
{
		bool bAnimCam = false;
		bAnimCam = tps->toggleAnimCam;

		bool bFovSlideMode = RE::PlayerControls::GetSingleton()->data.fovSlideMode;

		if (bAnimCam && !bFovSlideMode)
		{
			bool bFreeRotation = false;
			bFreeRotation = tps->freeRotationEnabled;

			tps->toggleAnimCam = false;
			tps->freeRotationEnabled = true;

			auto player = RE::PlayerCharacter::GetSingleton();
			int iState = 0;
			player->GetGraphVariableInt("iState", iState);
			if ( (iState > 0 && iState != 9) || player->IsSwimming())
			{
				player->data.angle.x -= tps->freeRotation.y;
				tps->freeRotation.y = 0.0f;
			}
			
			originUpdate(tps, a_nextState);

			tps->toggleAnimCam = bAnimCam;
			tps->freeRotationEnabled = bFreeRotation;
		}
		else
		{
			originUpdate(tps, a_nextState);
		}
	
}

void Hooks::LoadSettings()
{
	const int BUF_SIZE = 256;
	char buffer[BUF_SIZE];

	ForceFix = GetPrivateProfileInt("General", "EnabledDirectionFixMove", 0, INIPATH);

	GetPrivateProfileString("General", "fSideMoveRotationInputXMin", "0.7", buffer, BUF_SIZE, INIPATH);
	SideMoveRotationInputXMin = std::atof(buffer);

	GetPrivateProfileString("General", "fSideMoveRotationScale", "0.013", buffer, BUF_SIZE, INIPATH);
	SideMoveRotationScale = std::atof(buffer);

	GetPrivateProfileString("General", "fMovingTurnMax", "0.3", buffer, BUF_SIZE, INIPATH);
	MovingTurnMax = std::atof(buffer);

	GetPrivateProfileString("General", "fCommitmentTurnMax", "0.02", buffer, BUF_SIZE, INIPATH);
	CommitmentTurnMax = std::atof(buffer);

	PitchLookControl = GetPrivateProfileInt("General", "iPitchLookControl", 0, INIPATH);
}

void Hooks::Install()
{
	originProcessThumbstick =
		REL::Offset<uintptr_t>(RE::Offset::MovementHandler::Vtbl)
		.write_vfunc(0x2, ProcessThumbstickEx);
	originProcessButton =
		REL::Offset<uintptr_t>(RE::Offset::MovementHandler::Vtbl)
		.write_vfunc(0x4, ProcessButtonEx);

	if (PitchLookControl != 0)
	{
		originUpdate =
			REL::Offset<uintptr_t>(RE::Offset::ThirdPersonState::Vtbl)
			.write_vfunc(0x3, UpdateEx);
	}
}
