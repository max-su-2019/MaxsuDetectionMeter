#include "Hook.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	void ActorUpdateHook::ActorUpdate(RE::Actor* a_owner, float a_delta)
	{
		auto target = RE::PlayerCharacter::GetSingleton();
		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraRoot = camera ? camera->cameraRoot : nullptr;

		auto ctrlMap = RE::ControlMap::GetSingleton();

		if (target && target->IsSneaking() && a_owner && !a_owner->IsPlayerRef() && !a_owner->IsDead() && !a_owner->NotShowOnStealthMeter() && !a_owner->IsPlayerTeammate() && !a_owner->IsSummonedByPlayer() &&
			a_owner->currentProcess && a_owner->currentProcess->high && cameraRoot && ctrlMap && ctrlMap->IsSneakingControlsEnabled() && ctrlMap->IsMovementControlsEnabled()) {

			auto level = MeterHandler::ReCalculateDetectionLevel(a_owner->RequestDetectionLevel(target));
			auto stealthPoint = MeterHandler::GetStealthPoint(a_owner);
			const auto ownerID = a_owner->formID;
			auto meterHandler = MeterHandler::GetSingleton();

			auto CamTrans = RE::NiTransform(cameraRoot->world.rotate, target->GetPosition());
			auto const angle = CamTrans.GetHeadingAngle(a_owner->GetPosition());

			if (!meterHandler->meterArr.count(ownerID)) {
				if ( (!target->IsInCombat() && ((level >= meterHandler->meterSettings->minTriggerLevel.get_data() && a_owner->HasLOS(target)) || level >= 100)) ||
					(target->IsInCombat() && stealthPoint.has_value()) ) 
				{
					std::scoped_lock lock(meterHandler->m_mutex);	//thread mutex lock
					auto meterObj = std::make_shared<MeterObj>(angle);
					meterHandler->meterArr.emplace(ownerID, meterObj);
					logger::debug("Add a Meter ID : {:x}", ownerID);	
				}
			}
			else {
				auto it = meterHandler->meterArr.find(a_owner->formID);
				if (it != meterHandler->meterArr.end() && it->second.load()) {
					if (!it->second.load()->Update(a_owner,angle,level)) {
						it->second.load()->MarkForRemove();
					}
				}
			}
		}

		_ActorUpdate(a_owner, a_delta);
	}
}