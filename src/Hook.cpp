#include "Hook.h"
#include "DataHandler.h"

namespace MaxsuDetectionMeter
{
	std::int32_t DetectionLevelHook::RequestDetectionLevel(RE::Actor* a_owner, RE::Actor* a_target, RE::DETECTION_PRIORITY a_priority)
	{
		auto result = _RequestDetectionLevel(a_owner, a_target, a_priority);

		auto camera = RE::PlayerCamera::GetSingleton();
		auto cameraRoot = camera ? camera->cameraRoot : nullptr;

		if (a_owner && a_target && a_target->IsPlayerRef() && a_target->IsSneaking() && a_owner->currentProcess && a_owner->currentProcess->high && cameraRoot) {
			auto level = MeterHandler::ReCalculateDetectionLevel(result);
			auto stealthPoint = MeterHandler::GetStealthPoint(a_owner);
			const auto ownerID = a_owner->formID;
			auto meterHandler = MeterHandler::GetSingleton();

			auto CamTrans = RE::NiTransform(cameraRoot->world.rotate, a_target->GetPosition());
			auto const angle = CamTrans.GetHeadingAngle(a_owner->GetPosition());

			if (!meterHandler->meterArr.count(ownerID) && ((level >= meterHandler->minTriggerLevel && a_owner->HasLOS(a_target)) || level >= 100 || stealthPoint.has_value())) {
				auto meterObj = std::make_shared<MeterObj>(angle);
				meterHandler->meterArr.emplace(ownerID, meterObj);
				logger::debug("Add a Meter ID : {:x}", ownerID);
			}
		}
			
		return result;
	}


	void ActorUpdateHook::ActorUpdate(RE::Actor* a_actor, float a_delta)
	{
		_ActorUpdate(a_actor, a_delta);

		if (a_actor) {
			auto meterHandler = MeterHandler::GetSingleton();
			auto it = meterHandler->meterArr.find(a_actor->formID);
			if (it != meterHandler->meterArr.end() && it->second.load()) {
				if (!it->second.load()->Update(a_actor)) {
					it->second.load()->MarkForRemove();
				}
			}
		}
	}
}