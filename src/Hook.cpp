#include "Hook.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	void CharacterEx::Hook_Update(float a_delta)
	{
		func(this, a_delta);

		auto playerref = RE::PlayerCharacter::GetSingleton();

		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraState = camera ? camera->currentState : nullptr;
		auto cameraRoot = camera ? camera->cameraRoot : nullptr;

		auto ctrlMap = RE::ControlMap::GetSingleton();

		if (playerref && playerref->IsSneaking() && MeterHandler::ShouldDisplayMeter(this) &&
			cameraState && (cameraState->id == RE::CameraState::kFirstPerson || cameraState->id == RE::CameraState::kThirdPerson) && cameraRoot &&
			ctrlMap && ctrlMap->IsSneakingControlsEnabled() && ctrlMap->IsMovementControlsEnabled() && ctrlMap->contextPriorityStack.back() == RE::UserEvents::INPUT_CONTEXT_ID::kGameplay) {
			auto level = MeterHandler::ReCalculateDetectionLevel(RequestDetectionLevel(playerref));
			auto stealthPoint = MeterHandler::GetStealthPoint(this);
			auto meterHandler = MeterHandler::GetSingleton();

			RE::NiTransform CamTrans;
			CamTrans.rotate = cameraRoot->world.rotate;
			CamTrans.translate = playerref->GetPosition();

			auto const angle = CamTrans.GetHeadingAngle(GetPosition());

			if (!meterHandler->meterArr.count(formID)) {
				if ((!playerref->IsInCombat() && (meterHandler->DisplayForNonCombat(this, level, playerref))) || (playerref->IsInCombat() && stealthPoint.has_value())) {
					std::scoped_lock lock(meterHandler->m_mutex);  //thread mutex lock
					auto meterObj = std::make_shared<MeterObj>(angle);
					meterHandler->meterArr.emplace(formID, meterObj);
					DEBUG("Add a Meter ID : {:x}", formID);
				}
			} else {
				auto it = meterHandler->meterArr.find(formID);
				if (it != meterHandler->meterArr.end() && it->second.load() && !it->second.load()->Update(this, angle, level))
					it->second.load()->MarkForRemove();
			}
		}
	}
}
